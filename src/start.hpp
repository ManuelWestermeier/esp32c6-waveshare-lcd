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

Credentials start() {
  // === Mount LittleFS with retry ===
  while (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Failed to mount FS");
    delay(2000);
  }

  // === Ensure base directories exist ===
  if (!LittleFS.exists("/users")) LittleFS.mkdir("/users");
  if (!LittleFS.exists("/wifi")) LittleFS.mkdir("/wifi");

  // === Ask for username ===
  String username = ask("Enter your username", "");
  String userDir = "/users/" + username;
  String userPath = userDir + "/data.txt";
  String password;

  if (!LittleFS.exists(userPath)) {
    // New user
    LittleFS.mkdir(userDir);
    password = ask("Create a password", "");

    File userFile = LittleFS.open(userPath, "w");
    if (!userFile) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Error creating user file!");
      delay(3000);
      tft.fillScreen(UI_BG);
      return start();  // try again
    }
    userFile.println(username);
    userFile.println(password);
    userFile.close();
  } else {
    // Existing user
    bool passwordCorrect = false;
    while (!passwordCorrect) {
      File userFile = LittleFS.open(userPath, "r");
      if (!userFile) {
        tft.fillScreen(UI_BG);
        tft.setTextColor(UI_Text);
        tft.setCursor(20, 20);
        tft.println("Error reading user file!");
        delay(3000);
        tft.fillScreen(UI_BG);
        return start();
      }

      String storedUsername = userFile.readStringUntil('\n');
      String storedPassword = userFile.readStringUntil('\n');
      storedPassword.trim();
      userFile.close();

      password = ask("Enter your password", "");
      password.trim();

      if (password == storedPassword) {
        passwordCorrect = true;
      } else {
        tft.fillScreen(UI_BG);
        tft.setTextColor(UI_Text);
        tft.setCursor(20, 20);
        tft.println("Password incorrect.");
        delay(2000);
      }
    }
  }

  // === Versuche gespeicherte Wi-Fi-Netzwerke zu verbinden ===
  struct WifiEntry {
    String ssid;
    String pass;
  };
  std::vector<WifiEntry> savedNetworks;

  if (LittleFS.exists("/wifi/networks.txt")) {
    File netFile = LittleFS.open("/wifi/networks.txt", "r");
    while (netFile.available()) {
      String ssid = netFile.readStringUntil('\n');
      String pass = netFile.readStringUntil('\n');
      ssid.trim();
      pass.trim();
      if (ssid.length() > 0) {
        savedNetworks.push_back({ ssid, pass });
      }
    }
    netFile.close();
  }

  std::vector<int> connectedIndices;

  for (int i = 0; i < (int)savedNetworks.size(); i++) {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.printf("Trying Wi-Fi:\n%s\n", savedNetworks[i].ssid.c_str());

    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(savedNetworks[i].ssid.c_str(), savedNetworks[i].pass.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 8000) {
      delay(500);
      tft.print(".");
    }
    tft.println();

    if (WiFi.status() == WL_CONNECTED) {
      connectedIndices.push_back(i);
      tft.println("Connected!");
      delay(1000);
    } else {
      tft.println("Failed.");
      delay(1000);
    }
  }

  if (connectedIndices.size() == 1) {
    // Genau ein Netzwerk erfolgreich verbunden
    int idx = connectedIndices[0];
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.printf("Connected to:\n%s\n", savedNetworks[idx].ssid.c_str());
    delay(1000);
    return { username, password };
  } else if (connectedIndices.size() > 1) {
    // Mehrere Verbindungen erfolgreich - User wählt aus
    std::vector<String> options;
    for (int i : connectedIndices) {
      options.push_back(savedNetworks[i].ssid);
    }
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Multiple networks connected, choose one:");

    int chosen = select(options);
    if (chosen < 0 || chosen >= (int)options.size()) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("No valid selection.");
      delay(2000);
      // Einfach erstes nehmen
      chosen = 0;
    }

    int netIdx = connectedIndices[chosen];
    // Verbinde endgültig mit dem ausgewählten Netzwerk
    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(savedNetworks[netIdx].ssid.c_str(), savedNetworks[netIdx].pass.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      tft.print(".");
    }
    tft.println();

    if (WiFi.status() == WL_CONNECTED) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.printf("Connected to:\n%s\n", savedNetworks[netIdx].ssid.c_str());
      delay(1000);
      return { username, password };
    } else {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Failed to connect finally.");
      delay(2000);
      // Fallback weiter unten
    }
  }

  // === Kein gespeichertes Netzwerk funktionierte, neue Netzwerke scannen ===
  while (true) {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Scanning networks...");
    int n = WiFi.scanNetworks();
    std::vector<String> ssids;
    for (int i = 0; i < n; ++i) {
      ssids.push_back(WiFi.SSID(i));
    }

    int index = select(ssids);
    if (index == -1 || index >= (int)ssids.size()) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("No Wi-Fi selected.");
      delay(2000);
      continue;
    }

    String chosenSSID = ssids[index];
    String wifiPassword = ask("Password for " + chosenSSID, "");

    // In Netzwerke-Datei speichern (append, wenn noch nicht drin)
    bool alreadySaved = false;
    for (auto &entry : savedNetworks) {
      if (entry.ssid == chosenSSID) {
        entry.pass = wifiPassword;
        alreadySaved = true;
        break;
      }
    }
    if (!alreadySaved) {
      savedNetworks.push_back({ chosenSSID, wifiPassword });
    }
    // Schreibe alle Netzwerke zurück in Datei
    File wifiFile = LittleFS.open("/wifi/networks.txt", "w");
    if (!wifiFile) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Failed to save Wi-Fi");
      delay(2000);
      continue;
    }
    for (auto &entry : savedNetworks) {
      wifiFile.println(entry.ssid);
      wifiFile.println(entry.pass);
    }
    wifiFile.close();

    // Verbindung versuchen
    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(chosenSSID.c_str(), wifiPassword.c_str());

    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.print("Connecting to ");
    tft.println(chosenSSID);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      tft.print(".");
    }
    tft.println();

    if (WiFi.status() == WL_CONNECTED) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Connected!");
      tft.fillScreen(UI_BG);
      delay(1000);
      return { username, password };
    } else {
      tft.setCursor(20, 20);
      tft.println("Connection failed.");
      delay(2000);
    }
  }
}
