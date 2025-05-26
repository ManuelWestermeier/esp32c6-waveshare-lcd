#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <LittleFS.h>
#include <base64.h>
#include <AESLib.h>

#include "colors.hpp"
#include "metadata.hpp"
#include "start.hpp"
#include "input.hpp"

struct Request {
  String type;
  String data;
  String toString() {
    return type + "\n" + data;
  }
};

struct Browser {
  Credentials credentials;
  String appDomain = "";
  bool onPage = false;
  WiFiClient client;

  AESLib aesLib;
  byte aesKey[16];         // 128-bit key
  byte aesIV[16] = { 0 };  // Initialization Vector (can be random or constant)

  String encrypt(String msg) {
    int len = msg.length() + 1;
    char encrypted[len + AES_BLOCKLEN];  // ensure enough space
    aesLib.encrypt64(msg.c_str(), aesKey, aesIV, encrypted);
    return String(encrypted);
  }

  String decrypt(String encryptedBase64) {
    int len = encryptedBase64.length() + 1;
    char decrypted[len];
    aesLib.decrypt64(encryptedBase64.c_str(), aesKey, aesIV, decrypted);
    return String(decrypted);
  }

  void initAESKeyFromPassword(String password) {
    for (int i = 0; i < 16; i++) {
      aesKey[i] = i < password.length() ? password[i] : 0;
    }
  }

  void Start() {
    // appDomain = "hg2z.duckdns.org:25279";
    appDomain = "192.168.178.33:25279";
    // appDomain = "192.168.4.2:25279";
    onPage = false;
  }

  bool Connect() {
    int sep = appDomain.indexOf(":");
    if (sep == -1) {
      showError("Invalid domain!");
      return false;
    }

    String host = appDomain.substring(0, sep);
    int port = appDomain.substring(sep + 1).toInt();

    if (!client.connect(host.c_str(), port)) {
      showError("Connection failed");
      return false;
    }

    initAESKeyFromPassword(credentials.password);

    clearScreen(UI_BG);
    tft.setTextSize(2);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Connected!");
    tft.setCursor(0, 0);
    delay(1000);

    client.println("init");
    onPage = true;
    return true;
  }

  void HandleInput() {
    auto event = Input::getLastEvent();
    switch (event) {
      case Input::Click:
        client.println("click");
        break;
      case Input::LongPress:
        client.println("longclick");
        break;
      case Input::DoubleClick:
        client.println("dblclick");
        break;
      case Input::TripleClick:
        if (ok(" Exit App"))
          Start();
        else if (client.connected())
          client.println("rerender");
        break;
      default:
        break;
    }
  }

  void HandleServerMessages() {
    while (client.connected() && client.available()) {
      String cmd = client.readStringUntil('\n');

      // screen output
      if (cmd == "fillScreen") {
        uint16_t color = client.readStringUntil('\n').toInt();
        tft.fillScreen(color);
      } else if (cmd == "setCursor") {
        int x = client.readStringUntil('\n').toInt();
        int y = client.readStringUntil('\n').toInt();
        tft.setCursor(x, y);
      } else if (cmd == "write") {
        String text = client.readStringUntil('\n');
        tft.print(text);
      } else if (cmd == "println") {
        String text = client.readStringUntil('\n');
        tft.println(text);
      } else if (cmd == "drawPixel") {
        int x = client.readStringUntil('\n').toInt();
        int y = client.readStringUntil('\n').toInt();
        uint16_t color = client.readStringUntil('\n').toInt();
        tft.drawPixel(x, y, color);
      } else if (cmd == "fillRect") {
        int x = client.readStringUntil('\n').toInt();
        int y = client.readStringUntil('\n').toInt();
        int w = client.readStringUntil('\n').toInt();
        int h = client.readStringUntil('\n').toInt();
        uint16_t color = client.readStringUntil('\n').toInt();
        tft.fillRect(x, y, w, h, color);
      } else if (cmd == "setTextColor") {
        uint16_t color = client.readStringUntil('\n').toInt();
        tft.setTextColor(color);
      } else if (cmd == "setTextSize") {
        uint16_t size = client.readStringUntil('\n').toInt();
        tft.setTextSize(size);
      }
      // storage
      else if (cmd == "get-storage-key") {
        String key = client.readStringUntil('\n');
        String path = "/" + credentials.username + "/browser/storage/" + appDomain + "/" + key + ".data";
        File file = LittleFS.open(path, "r");
        String value = "-1";
        if (file) {
          String encrypted = file.readString();
          file.close();
          value = decrypt(encrypted);
        }
        if (client.connected())
          client.println("return-storage-key\n" + value);
      } else if (cmd == "set-storage-key") {
        String key = client.readStringUntil('\n');
        String value = client.readStringUntil('\n');
        String encrypted = encrypt(value);
        String path = "/" + credentials.username + "/browser/storage/" + appDomain + "/" + key + ".data";

        // Ensure directory exists
        String folderPath = "/" + credentials.username + "/browser/storage/" + appDomain;
        LittleFS.mkdir(folderPath);

        File file = LittleFS.open(path, "w");
        if (file) {
          file.print(encrypted);
          file.close();
        }
      }
      // input
      else if (cmd == "ask-text") {
        String question = client.readStringUntil('\n');
        String defaultValue = client.readStringUntil('\n');
        String value = ask(question, defaultValue);
        value.replace("\n", "\\n");
        if (client.connected())
          client.println("ask-text-value\n" + value);
        if (client.connected())
          client.println("rerender");
      } else if (cmd == "ask-ok") {
        String question = client.readStringUntil('\n');
        bool value = ok(question);
        if (client.connected())
          client.println(String("ask-ok-value\n") + (value ? "yes" : "no"));
        if (client.connected())
          client.println("rerender");
      } else if (cmd == "ask-select") {
        std::vector<String> options;
        for (uint8_t i = 0; i < 250; i++) {
          String option = client.readStringUntil('\n');
          if (option == "::OPTIONS_END::")
            break;
          options.push_back(option);
        }
        int result = select(options);
        if (client.connected())
          client.println("ask-select-value\n" + String(result));
        if (client.connected())
          client.println("rerender");
      }
    }
  }

  void Update() {
    if (WiFi.status() != WL_CONNECTED)
      return;

    if (appDomain.isEmpty())
      return Start();

    if (!onPage) {
      Connect();
    } else {
      if (!client.connected()) {
        clearScreen(UI_BG);
        tft.setTextColor(UI_Text);
        tft.setTextSize(2);
        tft.setCursor(20, 20);
        tft.println("Offline!");
        tft.setCursor(0, 0);
        delay(1000);
        Start();
        Connect();
      }
      HandleInput();
      HandleServerMessages();
    }
  }

private:
  void showError(const char *msg) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_RED);
    tft.println(msg);
  }

  void clearScreen(uint16_t color) {
    tft.fillScreen(color);
    tft.setCursor(0, 0);
  }
};
