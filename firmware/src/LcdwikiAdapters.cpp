#include "LcdwikiAdapters.h"
#ifdef BOARD_LCDWIKI_ES3C28P
#include "board_profiles/lcdwiki_es3c28p_candidate.h"
#include <SPI.h>
using namespace LcdwikiEs3c28p;
void Ili9341vDisplay::begin(){SPI.begin(lcdSclk,lcdMiso,lcdMosi,lcdCs);lcd_.begin();lcd_.setRotation(0);pinMode(backlight,OUTPUT);digitalWrite(backlight,HIGH);}
void Ili9341vDisplay::clear(uint16_t color){lcd_.fillScreen(color);}
void Ili9341vDisplay::text(int x,int y,const String& value,uint16_t color){lcd_.setCursor(x,y);lcd_.setTextColor(color);lcd_.setTextSize(2);lcd_.print(value);}
void Ili9341vDisplay::button(int x,int y,int w,int h,const String& label){lcd_.drawRect(x,y,w,h,ILI9341_BLACK);text(x+8,y+12,label,ILI9341_BLACK);}
void Ft6336gTouch::begin(){pinMode(touchReset,OUTPUT);digitalWrite(touchReset,LOW);delay(10);digitalWrite(touchReset,HIGH);pinMode(touchInterrupt,INPUT_PULLUP);Wire.begin(touchSda,touchScl);Wire.beginTransmission(touchAddress);present_=Wire.endTransmission()==0;}
bool Ft6336gTouch::read(int& x,int& y){if(!present_)return false;Wire.beginTransmission(touchAddress);Wire.write(0x02);if(Wire.endTransmission(false)!=0||Wire.requestFrom((int)touchAddress,5)!=5)return false;int count=Wire.read()&0x0f;int xh=Wire.read(),xl=Wire.read(),yh=Wire.read(),yl=Wire.read();if(count<1||count>2)return false;x=((xh&0x0f)<<8)|xl;y=((yh&0x0f)<<8)|yl;return x>=0&&x<width&&y>=0&&y<height;}
#endif
