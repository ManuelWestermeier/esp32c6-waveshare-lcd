#pragma once

#include "ask.hpp"
#include "select.hpp"

#include <Wifi.h>
#include <LittleFS>
#include <AES.h>

void start() {
  // setup the connection & fs
  // check for wifis in the arear
  // checks if the user has an account
  // if not ask the user & create a userdata file under /users/{name}/data.enc
  // it includes the password \n name (all encrypted with the password)
  // if the username exists check that the encrypted match and return or show an error
  // try to connect to the internet.
  // ask for wifi password with an ssid list.

  bool ok("Question");
  String ask(String question, String _default)
  int select(const std::vector<String>& originalOptions);
  //returns -1 if nothing selected

  //add loops and store the last value to get the correct user inputs
}