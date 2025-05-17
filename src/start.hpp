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

void start() {
  tft.setCursor(20, 20);
  // Mount LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    tft.println("Failed to mount LittleFS");
    return;
  }

  // Ensure users directory exists
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
    File userFile = LittleFS.open(userPath, "r");
    String storedPassword = userFile.readStringUntil('\n');
    storedPassword.trim();
    String storedUsername = userFile.readStringUntil('\n');
    userFile.close();
getPassword:
    password = ask("Enter your password", "");
    password.trim();

    if (strcmp(password.c_str(), storedPassword.c_str()) != 0) {
      tft.setCursor(0, 20);
      tft.println(" Password does\n not match.");
      delay(3000);
      goto getPassword;
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
    tft.setCursor(0, 20);
    tft.println("Connecting to stored Wi-Fi: ");
    tft.println(storedSSID);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      tft.print(".");
    }
    tft.println();
    if (WiFi.status() == WL_CONNECTED) {
      tft.println("\nConnected to Wi-Fi.");
      return;
    } else {
      tft.println("\nFailed to connect to stored Wi-Fi.");
    }
  }

wifiAsk:
  // Scan available networks
  Serial.println("Scanning Wi-Fi networks...");
  tft.println("Scanning Wi-Fi networks...");
  int n = WiFi.scanNetworks();
  std::vector<String> ssids;
  for (int i = 0; i < n; ++i) {
    ssids.push_back(WiFi.SSID(i));
  }

  // Ask for Wi-Fi network
  int index = select(ssids);
  if (index == -1) {
    Serial.println("No Wi-Fi selected.");
    goto wifiAsk;
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
    tft.println("\nConnected successfully.");
    return;
  } else {
    tft.println("\nFailed to connect.");
  }
  goto wifiAsk;
}