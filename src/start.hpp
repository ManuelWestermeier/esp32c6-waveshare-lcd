#pragma once

#include "metadata.hpp"
#include "ask.hpp"
#include "select.hpp"

#include <WiFi.h>
#include <LittleFS.h>

void start() {
  // Mount LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    tft.setCursor(20, 20);
    tft.println("Failed to mount LittleFS");
    return;
  }

  // Ensure users directory exists
  LittleFS.mkdir("/users");
  LittleFS.mkdir("/wifi");

  // Scan available networks
  Serial.println("Scanning Wi-Fi networks...");
  int n = WiFi.scanNetworks();
  std::vector<String> ssids;
  for (int i = 0; i < n; ++i) {
    ssids.push_back(WiFi.SSID(i));
  }

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
    File userFile = LittleFS.open(userPath, "r");
    String storedPassword = userFile.readStringUntil('\n');
    String storedUsername = userFile.readStringUntil('\n');
    userFile.close();
    password = ask("Enter your password", "");
    if (password != storedPassword) {
      Serial.println("Password does not match.");
      return;
    }
  }

  // Check for stored Wi-Fi credentials
  File wifiFile = LittleFS.open("/wifi/data.txt", "r");
  String storedSSID = "", storedPass = "";
  if (wifiFile) {
    storedSSID = wifiFile.readStringUntil('\n');
    storedPass = wifiFile.readStringUntil('\n');
    wifiFile.close();

    // Try to connect automatically
    WiFi.begin(storedSSID.c_str(), storedPass.c_str());
    Serial.print("Connecting to stored Wi-Fi: ");
    Serial.println(storedSSID);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi.");
      return;
    } else {
      Serial.println("\nFailed to connect to stored Wi-Fi.");
    }
  }

  // Ask for Wi-Fi network
  int index = select(ssids);
  if (index == -1) {
    Serial.println("No Wi-Fi selected.");
    return;
  }
  String chosenSSID = ssids[index];
  String wifiPassword = ask("Enter password for " + chosenSSID, "");

  // Store Wi-Fi credentials
  wifiFile = LittleFS.open("/wifi/data.txt", "w");
  wifiFile.println(chosenSSID);
  wifiFile.println(wifiPassword);
  wifiFile.close();

  // Connect to Wi-Fi
  WiFi.begin(chosenSSID.c_str(), wifiPassword.c_str());
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(chosenSSID);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected successfully.");
  } else {
    Serial.println("\nFailed to connect.");
  }
}
