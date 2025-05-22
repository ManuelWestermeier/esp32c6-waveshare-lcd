#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

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

  void Start() {
    appDomain = "hg2z.duckdns.org:25279";
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

    clearScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_GREEN);
    tft.println("Connected!");

    client.println("init\nconnected");  // NO USERNAME SENT
    onPage = true;
    return true;
  }

  void HandleInput() {
    auto event = Input::getLastEvent();

    switch (event) {
      case Input::Click:
        client.println("click\n" + String(millis()));
        break;
      case Input::DoubleClick:
        client.println("dblclick\n" + String(millis()));
        break;
      case Input::TripleClick:
        if (ok(" Exit App"))
          Start();  // Restart logic
        break;
      default:
        break;
    }
  }

  void HandleServerMessages() {
    while (client.connected() && client.available()) {
      String cmd = client.readStringUntil('\n');

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
      } else if (cmd == "ask-text") {
        String question = client.readStringUntil('\n');
        String defaultValue = client.readStringUntil('\n');
        String value = ask(question, defaultValue);
        if (client.connected()) {
          client.println("ask-text-value\n" + value);
        }
      } else if (cmd == "ask-ok") {
        String question = client.readStringUntil('\n');
        String defaultValue = client.readStringUntil('\n');
        bool value = ok(question);
        if (client.connected()) {
          client.println("ask-ok-value\n" + value ? "yes" : "no");
        }
      } else if (cmd == "ask-select") {
        std::vector<String> options;
        for (uint8_t index = 0; index < 256 &&; i++) {
          String option = client.readStringUntil('\n');
          options.push_back(option);
        }
        int result = select(options);
        if (client.connected()) {
          client.println("ask-select-value\n" + String(result));
        }
      }
    }
  }

  void Update() {
    if (WiFi.status() != WL_CONNECTED)
      return;

    if (appDomain.isEmpty())
      Start();

    if (!onPage) {
      Connect();
    } else {
      HandleInput();
      HandleServerMessages();
    }
  }

private:
  void showError(const char* msg) {
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
