#include "Display_ST7789.h"

void setup()
{       
  LCD_Init();
  LCD_SetCursor(10, 10, 50, 50);
  Backlight_Init();
  Set_Backlight(100);
}

void loop()
{
}
