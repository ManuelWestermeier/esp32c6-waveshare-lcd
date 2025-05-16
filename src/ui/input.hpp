#pragma once

#include "ui.hpp"
#include "../read-text.hpp"

struct Input : Ui {
  String text;
  void Render() {
    readText();
  };
  void Focus(){

  };
  void FocusEnd(){

  };
  void Click(){

  };
};
