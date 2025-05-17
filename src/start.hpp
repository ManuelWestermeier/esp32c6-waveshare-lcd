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
  tft.setCursor(20, 20);

  // Mount LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    tft.println("Failed to mount LittleFS");
    return { "", "" };
  }

  // Ensure necessary directories exist
  LittleFS.mkdir("/users");
  LittleFS.mkdir("/wifi");

  // Ask for username
  String username = ask("Enter your username", "");
  String userPath = "/users/" + username + "/data.txt";

  String password;

  if (!LittleFS.exists(userPath)) {
    LittleFS.mkdir("/users/" + username);
    password = ask("Create a password", "");
    File userFile = LittleFS.open(userPath, "w");
    userFile.println(password);
    userFile.println(username);
    userFile.close();
  } else {
    bool passwordCorrect = false;
    while (!passwordCorrect) {
      File userFile = LittleFS.open(userPath, "r");
      String storedPassword = userFile.readStringUntil('\n');
      storedPassword.trim();
      userFile.close();

      password = ask("Enter your password", "");
      password.trim();

      if (password == storedPassword) {
        passwordCorrect = true;
      } else {
        tft.setCursor(0, 20);
        tft.println(" Password does\n not match.");
        delay(3000);
      }
    }
  }

  // Check for stored Wi-Fi credentials
  File wifiFile = LittleFS.open("/wifi/data.txt", "r");
  if (wifiFile) {
    String storedSSID = wifiFile.readStringUntil('\n');
    String storedPass = wifiFile.readStringUntil('\n');
    wifiFile.close();

    storedSSID.trim();
    storedPass.trim();

    WiFi.begin(storedSSID.c_str(), storedPass.c_str());
    tft.setCursor(0, 20);
    tft.println("Connecting to stored Wi-Fi:");
    tft.println(storedSSID);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      tft.print(".");
    }
    tft.println();

    if (WiFi.status() == WL_CONNECTED) {
      tft.println("Connected to Wi-Fi.");
      return { username, password };
    } else {
      tft.println("Failed to connect to stored Wi-Fi.");
    }
  }

  // Ask for new Wi-Fi credentials
  while (true) {
    tft.println("Scanning Wi-Fi networks...");
    int n = WiFi.scanNetworks();
    std::vector<String> ssids;
    for (int i = 0; i < n; ++i) {
      ssids.push_back(WiFi.SSID(i));
    }

    int index = select(ssids);
    if (index == -1 || index >= ssids.size()) {
      tft.println("No Wi-Fi selected.");
      continue;
    }

    String chosenSSID = ssids[index];
    String wifiPassword = ask("Enter password for " + chosenSSID, "");

    wifiFile = LittleFS.open("/wifi/data.txt", "w");
    wifiFile.println(chosenSSID);
    wifiFile.println(wifiPassword);
    wifiFile.close();

    WiFi.begin(chosenSSID.c_str(), wifiPassword.c_str());
    tft.setCursor(0, 20);
    tft.print("Connecting to Wi-Fi: ");
    tft.println(chosenSSID);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      tft.print(".");
    }
    tft.println();

    if (WiFi.status() == WL_CONNECTED) {
      tft.println("Connected successfully.");
      return { username, password };
    } else {
      tft.println("Failed to connect.");
    }
  }
}
