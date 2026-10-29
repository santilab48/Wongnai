#pragma once
#include <WiFi.h>
#include "CaptureStorage.h"
class CaptureServer {
public:
 bool begin(uint16_t port,CaptureStorage* storage);void tick();uint32_t accepted()const{return accepted_;}uint32_t errors()const{return errors_;}bool receiving()const{return active_;}
private:
 WiFiServer* server_=nullptr;WiFiClient client_;CaptureStorage* storage_=nullptr;uint32_t accepted_=0,errors_=0,opened_=0;bool active_=false;std::vector<uint8_t> buffer_;void finish();
};
