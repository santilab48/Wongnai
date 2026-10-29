#pragma once
#include <Arduino.h>
#include <vector>
struct UiEvent {String id,ip,format;size_t bytes=0;uint64_t timestamp=0;bool incomplete=false;bool simulated=false;bool uploaded=false;};
enum class UiPage {LiveCapture,History,RawText,RawHex,WiFiSettings};
class DisplayAdapter {public:virtual ~DisplayAdapter()=default;virtual void clear(uint16_t color)=0;virtual void text(int x,int y,const String& value,uint16_t color)=0;virtual void button(int x,int y,int w,int h,const String& label)=0;};
class TouchAdapter {public:virtual ~TouchAdapter()=default;virtual bool read(int& x,int& y)=0;};
class UiModel {
public:
 UiPage page=UiPage::LiveCapture;std::vector<UiEvent> recent;String selectedId;std::vector<uint8_t> selectedRaw;
 String wifiSsid,wifiPassword;bool editingPassword=false;bool connectRequested=false;
 void setEvents(const std::vector<UiEvent>& events);
 void setRaw(const std::vector<uint8_t>& raw){selectedRaw=raw;}
 void draw(DisplayAdapter& display,const String& ssid,const String& ip,const String& wifiState,const String& listenerState,size_t pending,size_t freeBytes,size_t capacity);
 void onTouch(int x,int y);
};
