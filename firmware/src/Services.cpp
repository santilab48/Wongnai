#include "Services.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h>
#include <algorithm>
using std::min;
void WiFiManager::begin(){prefs_.begin("wifi",false);String s=prefs_.getString("ssid","");if(s.length()){WiFi.mode(WIFI_STA);WiFi.begin(s.c_str(),prefs_.getString("password","").c_str());}else{WiFi.mode(WIFI_AP);WiFi.softAP("Wongnai-Setup");}}
void WiFiManager::tick(){if(WiFi.getMode()==WIFI_STA&&WiFi.status()!=WL_CONNECTED&&millis()-retry_>10000){retry_=millis();WiFi.reconnect();}}
String WiFiManager::ssid()const{return WiFi.SSID();}
bool WiFiManager::configure(const String& s,const String& p){if(!s.length())return false;prefs_.putString("ssid",s);prefs_.putString("password",p);WiFi.mode(WIFI_STA);WiFi.begin(s.c_str(),p.c_str());return true;}
String Diagnostics::state(bool listening,bool receiving,size_t freeBytes,bool storageAvailable)const{if(receiving)return "RECEIVING";if(!listening)return "OFFLINE";if(!storageAvailable)return "STORAGE OFFLINE";if(freeBytes<70000)return "STORAGE FULL";return "LISTENING";}
void Diagnostics::printHardwareReport()const{
 const auto flashMode=ESP.getFlashChipMode();
 const char* flashModeName="unknown";
 switch(flashMode){
  case FM_QIO:flashModeName="QIO";break;
  case FM_QOUT:flashModeName="QOUT";break;
  case FM_DIO:flashModeName="DIO";break;
  case FM_DOUT:flashModeName="DOUT";break;
  case FM_FAST_READ:flashModeName="FAST_READ";break;
  case FM_SLOW_READ:flashModeName="SLOW_READ";break;
  default:break;
 }
 const bool psramInitialized=psramFound();
 Serial.println("Hardware diagnostics:");
 Serial.printf("Chip: %s, revision %u, cores %u\n",ESP.getChipModel(),(unsigned)ESP.getChipRevision(),(unsigned)ESP.getChipCores());
 Serial.printf("Flash: %lu bytes, mode %s, bus frequency %lu MHz\n",(unsigned long)ESP.getFlashChipSize(),flashModeName,(unsigned long)ESP.getFlashFrequencyMHz());
 Serial.printf("PSRAM initialization: %s\n",psramInitialized?"success":"failure");
 Serial.printf("PSRAM total: %lu bytes\n",(unsigned long)ESP.getPsramSize());
 Serial.printf("Free heap: %lu bytes\n",(unsigned long)ESP.getFreeHeap());
}
void GoogleSheetsUploader::begin(CaptureStorage* s){storage_=s;prefs_.begin("upload",true);}
void GoogleSheetsUploader::tick(){if(WiFi.status()!=WL_CONNECTED||!storage_||millis()<retry_)return;String endpoint=prefs_.getString("url","");String token=prefs_.getString("token","");String ca=prefs_.getString("ca","");if(!endpoint.startsWith("https://")||!token.length()||!ca.length())return;auto jobs=storage_->uploadQueue(100);if(jobs.empty())return;auto r=jobs.back();std::vector<uint8_t> raw;if(!storage_->raw(r.id,raw))return;size_t len=0;std::vector<unsigned char> b64(4*((raw.size()+2)/3)+1);if(mbedtls_base64_encode(b64.data(),b64.size(),&len,raw.data(),raw.size()))return;JsonDocument j;j["eventId"]=r.id;j["timestamp"]=r.timestamp;j["deviceId"]=String((uint32_t)ESP.getEfuseMac(),HEX);j["sourceIp"]=r.ip;j["sourcePort"]=r.port;j["bytes"]=r.bytes;j["format"]=r.format;j["parsedOrder"]=r.text;j["rawBase64"]=String((char*)b64.data(),len);j["token"]=token;String body;serializeJson(j,body);WiFiClientSecure c;c.setCACert(ca.c_str());HTTPClient http;http.setTimeout(5000);if(!http.begin(c,endpoint)){retry_=millis()+delay_;return;}http.addHeader("Content-Type","application/json");int code=http.POST(body);String answer=http.getString();http.end();JsonDocument ack;if(code==200&&!deserializeJson(ack,answer)&&ack["ok"].as<bool>()&&ack["eventId"].as<String>()==r.id){storage_->acknowledge(r.id);delay_=1000;}else delay_=min<uint32_t>(delay_*2,300000u);retry_=millis()+delay_;}
void EmailReporter::begin(){prefs_.begin("email",true);}
void EmailReporter::tick(const Diagnostics& d){
 if(WiFi.status()!=WL_CONNECTED||millis()-last_<delay_)return;
 last_=millis();String endpoint=prefs_.getString("url","");String token=prefs_.getString("token","");String ca=prefs_.getString("ca","");
 String recipient=prefs_.getString("recipient","");if(!endpoint.startsWith("https://")||!token.length()||!ca.length()||!recipient.length())return;
 WiFiClientSecure client;client.setCACert(ca.c_str());HTTPClient http;http.setTimeout(5000);
 if(!http.begin(client,endpoint)){delay_=min<uint32_t>(delay_*2,3600000u);return;}
 JsonDocument req;req["action"]="report";req["token"]=token;req["recipient"]=recipient;req["errors"]=d.errors;
 String body;serializeJson(req,body);http.addHeader("Content-Type","application/json");int code=http.POST(body);String response=http.getString();http.end();JsonDocument ack;
 if(code==200&&!deserializeJson(ack,response)&&ack["ok"].as<bool>()&&ack["action"].as<String>()=="report")delay_=3600000;
 else delay_=min<uint32_t>(delay_*2,3600000u);
}
void DisplayManager::begin(){
 uiMutex_=xSemaphoreCreateMutex();
 if(!uiMutex_){Serial.println("Display UI mutex creation failed");return;}
#ifdef BOARD_LCDWIKI_ES3C28P
 adapter_.begin();
#else
 Serial.println("LCD adapter disabled: select LCDWIKI profile for candidate hardware");
#endif
}
void DisplayManager::onTouch(int x,int y){
 if(!uiMutex_||xSemaphoreTake(uiMutex_,portMAX_DELAY)!=pdTRUE)return;
 const UiPage oldPage=model_.page;
 model_.onTouch(x,y);
 if(model_.page!=oldPage)lastRefresh_=0;
 xSemaphoreGive(uiMutex_);
}
bool DisplayManager::takeConnectRequest(String& ssid,String& password){
 if(!uiMutex_||xSemaphoreTake(uiMutex_,portMAX_DELAY)!=pdTRUE)return false;
 const bool requested=model_.connectRequested;
 if(requested){ssid=model_.wifiSsid;password=model_.wifiPassword;model_.connectRequested=false;}
 xSemaphoreGive(uiMutex_);
 return requested;
}
void DisplayManager::tick(const Diagnostics& d,CaptureStorage& s,const String& listener){
#ifdef BOARD_LCDWIKI_ES3C28P
 if(!uiMutex_||xSemaphoreTake(uiMutex_,portMAX_DELAY)!=pdTRUE)return;
 uint32_t now=millis();if(now-lastDraw_<200){xSemaphoreGive(uiMutex_);return;}lastDraw_=now;
#ifdef LCDWIKI_DIAGNOSTIC
 if(!diagnosticDrawn_){adapter_.clear(0x001f);adapter_.text(8,8,"LCDWIKI DIAGNOSTIC",0xffff);adapter_.text(8,40,"Touch: tap screen",0xffff);diagnosticDrawn_=true;}
 xSemaphoreGive(uiMutex_);
#else
 const bool refresh=now-lastRefresh_>=2000;
 if(refresh)lastRefresh_=now;
 const UiPage requestedPage=model_.page;
 const String selectedId=model_.selectedId;
 size_t pending=pendingCache_,freeBytes=freeCache_,capacity=capacityCache_;
 xSemaphoreGive(uiMutex_);
 std::vector<UiEvent> events;
 if(refresh){
  pending=s.pending();freeBytes=s.freeBytes();capacity=s.capacity();
  auto records=s.history(4);
  for(const auto&r:records)events.push_back({r.id,r.ip,r.format,r.bytes,r.timestamp,r.incomplete,false,r.uploaded});
 }
 const bool loadRaw=selectedId!=loadedId_;
 std::vector<uint8_t> raw;
 if(loadRaw&&!selectedId.isEmpty())s.raw(selectedId,raw);
 if(xSemaphoreTake(uiMutex_,portMAX_DELAY)!=pdTRUE)return;
 if(refresh&&model_.page==requestedPage){
  pendingCache_=pending;freeCache_=freeBytes;capacityCache_=capacity;model_.setEvents(events);
  const String newestId=events.empty()?"":events.front().id;
  if(!hasLoggedUiState_||lastLoggedRecordCount_!=events.size()||lastLoggedPage_!=requestedPage||lastLoggedNewestId_!=newestId){
   Serial.printf("[UI] archive records=%u page=%s newest=%s\n",(unsigned)events.size(),requestedPage==UiPage::History?"history":"live",newestId.length()?newestId.c_str():"none");
   lastLoggedRecordCount_=events.size();lastLoggedPage_=requestedPage;lastLoggedNewestId_=newestId;hasLoggedUiState_=true;
  }
 }
 else if(refresh)lastRefresh_=0;
 if(loadRaw&&model_.selectedId==selectedId){model_.setRaw(raw);loadedId_=selectedId;}
 String wifiState=WiFi.status()==WL_CONNECTED?"CONNECTED":WiFi.getMode()==WIFI_AP?"SETUP AP":"CONNECTING";
 model_.draw(adapter_,WiFi.SSID(),WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():WiFi.softAPIP().toString(),wifiState,listener,pendingCache_,freeCache_,capacityCache_);
 xSemaphoreGive(uiMutex_);
#endif
#else
 (void)d;(void)s;(void)listener;
#endif
}
void TouchInput::begin(){
#ifdef BOARD_LCDWIKI_ES3C28P
 adapter_.begin();Serial.println(adapter_.present()?"FT6336G detected":"FT6336G not detected");
#endif
}
void TouchInput::tick(){
#ifdef BOARD_LCDWIKI_ES3C28P
 int x,y;bool down=adapter_.read(x,y);
 if(down&&!pressed_){
#ifdef LCDWIKI_DIAGNOSTIC
  Serial.printf("Touch: %d,%d\n",x,y);
#else
  if(display_)display_->onTouch(x,y);
#endif
 }
 pressed_=down;
#endif
}
