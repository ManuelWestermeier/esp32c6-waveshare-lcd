#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "colors.hpp"
#include "metadata.hpp"
#include "start.hpp"
#include "input.hpp"

struct Request
{
  String type;
  String data;
  String toString()
  {
    return type + "\n" + data;
  }
};

struct Browser
{
  Credentials credentials;
  String appDomain = "";
  bool onPage = false;
  WiFiClient client;

  void Start()
  {
    appDomain = "hg2z.duckdns.org:25279";
  }

  void Connect()
  {
    int sep = appDomain.indexOf(":");
    if (sep == -1)
    {
      tft.setCursor(0, 0);
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextColor(ST77XX_RED);
      tft.println("Invalid domain!");
      return;
    }

    String host = appDomain.substring(0, sep);
    int port = appDomain.substring(sep + 1).toInt();

    if (!client.connect(host.c_str(), port))
    {
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 0);
      tft.setTextColor(ST77XX_RED);
      tft.println("Conn. failed");
      return;
    }

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_GREEN);
    tft.println("Connected!");

    client.println("init\n" + credentials.username);
    onPage = true;
  }

  void HandleInput()
  {
    auto event = Input::getLastEvent();

    if (event == Input::Click)
    {
      client.println("click\n" + String(millis()));
      tft.setTextColor(ST77XX_WHITE);
      tft.println("Click sent");
    }
    else if (event == Input::DoubleClick)
    {
      client.println("dblclick\n" + String(millis()));
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("DoubleClick sent");
    }
    else if (event == Input::TripleClick)
    {
      client.println("tripleclick\n" + String(millis()));
      tft.setTextColor(ST77XX_CYAN);
      tft.println("TripleClick sent");
    }
  }

  void HandleServerMessages()
  {
    while (client.connected() && client.available())
    {
      String cmd = client.readStringUntil('\n');

      if (cmd == "fillScreen")
      {
        String color = client.readStringUntil('\n');
        tft.fillScreen(color.toInt());
      }
      else if (cmd == "setCursor")
      {
        int x = client.readStringUntil('\n').toInt();
        int y = client.readStringUntil('\n').toInt();
        tft.setCursor(x, y);
      }
      else if (cmd == "write")
      {
        String text = client.readStringUntil('\n');
        tft.setTextColor(ST77XX_WHITE);
        tft.print(text);
      }
    }
  }

  void Update()
  {
    if (WiFi.status() != WL_CONNECTED)
      return;
    if (appDomain.isEmpty())
      Start();

    if (!onPage)
    {
      Connect();
    }
    else
    {
      HandleInput();
      HandleServerMessages();
    }
  }
};
