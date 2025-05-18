#pragma once
#include "metadata.hpp"
#include "ask.hpp"
#include "select.hpp"

#include <WiFi.h>
#include <LittleFS.h>
#include "mbedtls/sha256.h"

struct Credentials {
  String username;      // ORIGINAL username (plain)
  String password;      // ORIGINAL password (plain)
  String usernameHash;  // sha256(username)
  String passwordHash;  // sha256(password)
};

// Compute SHA256(input) → hex string
String sha256Hex(const String& input) {
  uint8_t digest[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  // 0 = use SHA‑256 (not SHA‑224)
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx,
                        reinterpret_cast<const uint8_t*>(input.c_str()),
                        input.length());
  mbedtls_sha256_finish(&ctx, digest);
  mbedtls_sha256_free(&ctx);

  char out[65];
  for (int i = 0; i < 32; ++i) {
    sprintf(out + i * 2, "%02x", digest[i]);
  }
  out[64] = '\0';
  return String(out);
}

void mountFS() {
  while (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Failed to mount FS");
    delay(2000);
  }
  if (!LittleFS.exists("/users")) LittleFS.mkdir("/users");
  if (!LittleFS.exists("/wifi")) LittleFS.mkdir("/wifi");
}

// --- ACCOUNT CREATION & LOGIN ---

Credentials createAccount(const String& username, const String& /*userPath*/) {
  String password = ask("Create a password", "");
  String uHash = sha256Hex(username);
  String pHash = sha256Hex(password);

  // Store: original username, original password, then hashes
  String userDir = "/users/" + uHash;
  String userPath = userDir + "/data.txt";
  File f = LittleFS.open(userPath, "w");
  if (!f) {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Error creating user file!");
    delay(3000);
    // retry
    return createAccount(username, userPath);
  }

  f.println(username);  // line 1: orig username
  f.println(password);  // line 2: orig password
  f.println(uHash);     // line 3: sha256(username)
  f.println(pHash);     // line 4: sha256(password)
  f.close();

  return { username, password, uHash, pHash };
}

Credentials login(const String& username, const String& /*userPath*/) {
  String uHash = sha256Hex(username);
  String userDir = "/users/" + uHash;
  String userPath = userDir + "/data.txt";

  while (true) {
    File f = LittleFS.open(userPath, "r");
    if (!f) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Error reading user file!");
      delay(3000);
      continue;
    }

    // Read all four lines
    String origUser = f.readStringUntil('\n');
    origUser.trim();
    String origPass = f.readStringUntil('\n');
    origPass.trim();
    String storedUHash = f.readStringUntil('\n');
    storedUHash.trim();
    String storedPHash = f.readStringUntil('\n');
    storedPHash.trim();
    f.close();

    // Sanity check: file's hash matches computed hash
    if (storedUHash != uHash) {
      Serial.println("Username hash mismatch!");
      return { "", "", "", "" };
    }

    String pw = ask("Enter your password", "");
    pw.trim();
    String pwHash = sha256Hex(pw);

    if (pwHash == storedPHash) {
      // Success: return both plain & hashed
      return { origUser, origPass, storedUHash, storedPHash };
    }

    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Password incorrect.");
    delay(2000);
  }
}

// --- WIFI NETWORK STORAGE & CONNECT (unchanged hashing logic) ---

std::vector<std::pair<String, String>> loadSavedNetworks() {
  std::vector<std::pair<String, String>> nets;
  if (!LittleFS.exists("/wifi/networks.txt")) return nets;
  File f = LittleFS.open("/wifi/networks.txt", "r");
  while (f.available()) {
    String ssidH = f.readStringUntil('\n');
    ssidH.trim();
    String pw = f.readStringUntil('\n');
    pw.trim();
    if (ssidH.length()) nets.emplace_back(ssidH, pw);
  }
  f.close();
  return nets;
}

bool tryConnect(const String& ssid, const String& pw) {
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(ssid.c_str(), pw.c_str());
  tft.fillScreen(UI_BG);
  tft.setCursor(0, 20);
  tft.printf("Connect to:\n %s\n\n", ssid.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    delay(500);
    tft.print(".");
  }
  return (WiFi.status() == WL_CONNECTED);
}

bool scanAndConnect(std::vector<std::pair<String, String>>& saved) {
  tft.fillScreen(UI_BG);
  tft.setTextColor(UI_Text);
  tft.setCursor(20, 20);
  tft.println("Scanning networks...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    tft.println("No networks found.");
    delay(2000);
    return false;
  }

  std::vector<String> ssids;
  for (int i = 0; i < n; ++i) ssids.push_back(WiFi.SSID(i));
  int idx = select(ssids);
  if (idx < 0 || idx >= ssids.size()) return false;

  String chosen = ssids[idx];
  String chosenH = sha256Hex(chosen);
  String password;

  // Lookup by hashed SSID
  for (auto& net : saved) {
    if (net.first == chosenH) {
      password = net.second;
      if (tryConnect(chosen, password)) return true;
      break;
    }
  }

  // Ask if unknown or failed
  password = ask("Password for " + chosen, "");

  // Update or append
  bool found = false;
  for (auto& net : saved) {
    if (net.first == chosenH) {
      net.second = password;
      found = true;
      break;
    }
  }
  if (!found) saved.emplace_back(chosenH, password);

  // Persist
  File f = LittleFS.open("/wifi/networks.txt", "w");
  if (f) {
    for (auto& net : saved) {
      f.println(net.first);
      f.println(net.second);
    }
    f.close();
  }

  return tryConnect(chosen, password);
}

// --- STARTUP ---

Credentials start() {
  mountFS();
  String username = ask("Enter your username", "");
  String uHash = sha256Hex(username);
  String userDir = "/users/" + uHash;
  String userPath = userDir + "/data.txt";

  Credentials creds;
  if (!LittleFS.exists(userPath)) {
    LittleFS.mkdir(userDir);
    creds = createAccount(username, userPath);
  } else {
    creds = login(username, userPath);
  }

  auto savedNetworks = loadSavedNetworks();
  while (!scanAndConnect(savedNetworks)) {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Connection failed.");
    delay(2000);
  }
  tft.fillScreen(UI_BG);
  return creds;
}
