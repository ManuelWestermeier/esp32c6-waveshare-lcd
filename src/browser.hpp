#pragma once

#include <Arduino.h>

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
  String appDomain;
  bool onPage = false;

  void Start() {
    appDomain = "hg2z.duckdns.org:25279";
  }

  void Connect() {
  }

  void Update() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (appDomain.empty()) {
      return Start();
    }
    if (!onPage) {
      Connect()
    }
  }
};