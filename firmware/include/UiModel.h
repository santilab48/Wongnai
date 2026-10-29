#pragma once
#include <Arduino.h>
#include <vector>
struct UiEvent {String id,ip,format,preview;size_t bytes;uint64_t timestamp;bool uploaded;};
enum class UiPage {Dashboard,LiveCapture,RawText,RawHex,History,WiFiSettings,Settings};
class DisplayAdapter {public:virtual ~DisplayAdapter()=default;virtual void clear(uint16_t color)=0;virtual void text(int x,int y,const String& value,uint16_t color)=0;virtual void button(int x,int y,int w,int h,const String& label)=0;};
class TouchAdapter {public:virtual ~TouchAdapter()=default;virtual bool read(int& x,int& y)=0;};
class UiModel {
public:
 UiPage page=UiPage::Dashboard;std::vector<UiEvent> recent;
 void add(const UiEvent& event){recent.insert(recent.begin(),event);if(recent.size()>20)recent.pop_back();}
 void draw(DisplayAdapter& display,const String& ssid,const String& ip,const String& state,size_t pending);
 void onTouch(int x,int y);
};
