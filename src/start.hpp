#pragma once
#include "metadata.hpp"
#include "ask.hpp"
#include "select.hpp"

#include <WiFi.h>
#include <LittleFS.h>

struct Credentials {
  String username;
  String password;
};

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

Credentials createAccount(const String& username, const String& userPath) {
  String password = ask("Create a password", "");
  File userFile = LittleFS.open(userPath, "w");
  if (!userFile) {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Error creating user file!");
    delay(3000);
    tft.fillScreen(UI_BG);
    return createAccount(username, userPath);
  }
  userFile.println(username);
  userFile.println(password);
  userFile.close();
  return { username, password };
}

Credentials login(const String& username, const String& userPath) {
  while (true) {
    File userFile = LittleFS.open(userPath, "r");
    if (!userFile) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Error reading user file!");
      delay(3000);
      tft.fillScreen(UI_BG);
      return login(username, userPath);
    }

    String storedUsername = userFile.readStringUntil('\n');
    String storedPassword = userFile.readStringUntil('\n');
    storedPassword.trim();
    userFile.close();

    String password = ask("Enter your password", "");
    password.trim();

    if (password == storedPassword) {
      return { username, password };
    }

    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Password incorrect.");
    delay(2000);
  }
}

std::vector<std::pair<String, String>> loadSavedNetworks() {
  std::vector<std::pair<String, String>> networks;
  if (!LittleFS.exists("/wifi/networks.txt")) return networks;

  File netFile = LittleFS.open("/wifi/networks.txt", "r");
  while (netFile.available()) {
    String ssid = netFile.readStringUntil('\n');
    String pass = netFile.readStringUntil('\n');
    ssid.trim();
    pass.trim();
    if (ssid.length() > 0) {
      networks.emplace_back(ssid, pass);
    }
  }
  netFile.close();
  return networks;
}

bool tryConnect(const String& ssid, const String& pass) {
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());

  tft.fillScreen(UI_BG);
  tft.setCursor(0, 20);

  tft.println(" Connect to:\n " + ssid);
  tft.println();

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    delay(500);
    tft.print(".");
  }

  return WiFi.status() == WL_CONNECTED;
}

bool scanAndConnect(std::vector<std::pair<String, String>>& savedNetworks) {
  tft.fillScreen(UI_BG);
  tft.setTextColor(UI_Text);
  tft.setCursor(20, 20);
  tft.println("Scanning networks...");
  int n = WiFi.scanNetworks();

  std::vector<String> ssids;
  for (int i = 0; i < n; ++i) ssids.push_back(WiFi.SSID(i));
  if (ssids.empty() || n == 0) {
    tft.println("No networks found.");
    delay(2000);
    return false;
  }

  int index = select(ssids);
  if (index == -1 || index >= (int)ssids.size()) return false;

  String chosenSSID = ssids[index];
  String password;

  bool known = false;
  for (auto& net : savedNetworks) {
    if (net.first == chosenSSID) {
      password = net.second;
      if (tryConnect(chosenSSID, password)) return true;
      break;
    }
  }

  // Ask for password
  password = ask("Password for " + chosenSSID, "");

  // Update or add
  bool updated = false;
  for (auto& net : savedNetworks) {
    if (net.first == chosenSSID) {
      net.second = password;
      updated = true;
      break;
    }
  }
  if (!updated) savedNetworks.emplace_back(chosenSSID, password);

  // Save back to file
  File file = LittleFS.open("/wifi/networks.txt", "w");
  if (file) {
    for (auto& net : savedNetworks) {
      file.println(net.first);
      file.println(net.second);
    }
    file.close();
  }

  return tryConnect(chosenSSID, password);
}

Credentials start() {
  mountFS();

  String username = ask("Enter your username", "");
  String userDir = "/users/" + username;
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
