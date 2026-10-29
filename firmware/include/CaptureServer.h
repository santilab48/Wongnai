#pragma once
#include <WiFi.h>
#include "CaptureStorage.h"
#include "StreamAssembler.h"
class CaptureServer {
public:
 static constexpr size_t MaxClients=4, MaxJobBytes=32768;
 bool begin(uint16_t port,CaptureStorage* storage);void tick();
 uint32_t accepted()const{return accepted_;}uint32_t errors()const{return errors_;}
 bool receiving()const{return clients_[0].active||clients_[1].active||clients_[2].active||clients_[3].active;}
private:
 struct ClientState {WiFiClient socket;StreamAssembler bytes{MaxJobBytes};uint32_t last=0,sequence=0;bool active=false;};
 WiFiServer* server_=nullptr;ClientState clients_[MaxClients];CaptureStorage* storage_=nullptr;
 uint32_t accepted_=0,errors_=0;
 void finish(ClientState& c,bool incomplete);
};
