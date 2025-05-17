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
  // === Try to connect to stored Wi-Fi ===
  if (LittleFS.exists("/wifi/data.txt")) {
    File wifiFile = LittleFS.open("/wifi/data.txt", "r");
    String storedSSID = wifiFile.readStringUntil('\n');
    String storedPass = wifiFile.readStringUntil('\n');
    wifiFile.close();

    storedSSID.trim();
    storedPass.trim();

    WiFi.begin(storedSSID.c_str(), storedPass.c_str());

    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(0, 20);
    tft.println("Connecting to Wi-Fi:");
    tft.println(storedSSID);

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
      delay(1000);
      return { username, password };
    } else {
      // Disconnect and clear WiFi state after failed attempt
      WiFi.disconnect(true);

      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Stored Wi-Fi failed.");
      delay(2000);
      // Now will continue to ask for new Wi-Fi credentials
    }
  }

  // === Ask user for new Wi-Fi credentials ===
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
    if (index == -1 || index >= ssids.size()) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("No Wi-Fi selected.");
      delay(2000);
      continue;
    }

    String chosenSSID = ssids[index];
    String wifiPassword = ask("Password for " + chosenSSID, "");

    File wifiFile = LittleFS.open("/wifi/data.txt", "w");
    if (!wifiFile) {
      tft.fillScreen(UI_BG);
      tft.setTextColor(UI_Text);
      tft.setCursor(20, 20);
      tft.println("Failed to save Wi-Fi");
      delay(2000);
      continue;
    }
    wifiFile.println(chosenSSID);
    wifiFile.println(wifiPassword);
    wifiFile.close();

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
      delay(1000);
      return { username, password };
    } else {
      tft.setCursor(20, 20);
      tft.println("Connection failed.");
      delay(2000);
    }
  }
}
