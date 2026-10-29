#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "CaptureStorage.h"
class WiFiManager {public: void begin();void tick();String ssid()const;bool configure(const String& ssid,const String& password);private:Preferences prefs_;uint32_t retry_=0;};
class TimeManager {public:void begin(){configTime(0,0,"pool.ntp.org");} bool synced()const{return time(nullptr)>1700000000;}};
class Diagnostics {public:uint32_t errors=0;String state(bool listening,bool receiving,size_t freeBytes)const;};
class GoogleSheetsUploader {public:void begin(CaptureStorage* s);void tick();private:CaptureStorage* storage_=nullptr;Preferences prefs_;uint32_t retry_=0,delay_=1000;};
class EmailReporter {public:void begin();void tick(const Diagnostics& d);private:Preferences prefs_;uint32_t last_=0;};
class DisplayManager {public:void begin();void tick(const Diagnostics& d,const CaptureStorage& s);};
class TouchInput {public:void begin();void tick();};
