#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "CaptureStorage.h"
#include "UiModel.h"
#include "LcdwikiAdapters.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
class WiFiManager {public: void begin();void tick();String ssid()const;bool configure(const String& ssid,const String& password);private:Preferences prefs_;uint32_t retry_=0;};
class TimeManager {public:void begin(){configTime(0,0,"pool.ntp.org");} bool synced()const{return time(nullptr)>1700000000;}};
class Diagnostics {public:uint32_t errors=0;String state(bool listening,bool receiving,size_t freeBytes,bool storageAvailable)const;void printHardwareReport()const;};
class GoogleSheetsUploader {public:void begin(CaptureStorage* s);void tick();private:CaptureStorage* storage_=nullptr;Preferences prefs_;uint32_t retry_=0,delay_=1000;};
class EmailReporter {public:void begin();void tick(const Diagnostics& d);private:Preferences prefs_;uint32_t last_=0,delay_=60000;};
class DisplayManager {public:void begin();void tick(const Diagnostics& d,CaptureStorage& s,const String& listener);void onTouch(int x,int y);bool takeConnectRequest(String& ssid,String& password);private:UiModel model_;SemaphoreHandle_t uiMutex_=nullptr;uint32_t lastDraw_=0,lastRefresh_=0;size_t pendingCache_=0,freeCache_=0,capacityCache_=0;String loadedId_,lastLoggedNewestId_;size_t lastLoggedRecordCount_=0;UiPage lastLoggedPage_=UiPage::LiveCapture;bool hasLoggedUiState_=false,diagnosticDrawn_=false;
#ifdef BOARD_LCDWIKI_ES3C28P
Ili9341vDisplay adapter_;
#endif
};
class TouchInput {public:void begin();void tick();void attach(DisplayManager* display){display_=display;}private:DisplayManager* display_=nullptr;bool pressed_=false;
#ifdef BOARD_LCDWIKI_ES3C28P
Ft6336gTouch adapter_;
#endif
};
