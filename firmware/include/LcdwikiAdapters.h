#pragma once
#include "UiModel.h"
#ifdef BOARD_LCDWIKI_ES3C28P
#include <Adafruit_ILI9341.h>
#include <Wire.h>
class Ili9341vDisplay : public DisplayAdapter {
public:
 void begin();
 void clear(uint16_t color) override;
 void text(int x,int y,const String& value,uint16_t color) override;
 void button(int x,int y,int w,int h,const String& label) override;
private:
 Adafruit_ILI9341 lcd_{10,46,-1};
};
class Ft6336gTouch : public TouchAdapter {
public:
 void begin();
 bool read(int& x,int& y) override;
 bool present() const {return present_;}
private:
 bool present_=false;
};
#endif
