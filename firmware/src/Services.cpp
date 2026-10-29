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
String Diagnostics::state(bool listening,bool receiving,size_t freeBytes)const{if(freeBytes<70000)return "STORAGE FULL";if(receiving)return "RECEIVING";if(!listening)return "OFFLINE";return "LISTENING";}
void GoogleSheetsUploader::begin(CaptureStorage* s){storage_=s;prefs_.begin("upload",true);}
void GoogleSheetsUploader::tick(){if(WiFi.status()!=WL_CONNECTED||!storage_||millis()<retry_)return;String endpoint=prefs_.getString("url","");String token=prefs_.getString("token","");String ca=prefs_.getString("ca","");if(!endpoint.startsWith("https://")||!token.length()||!ca.length())return;auto jobs=storage_->history(100);if(jobs.empty())return;auto r=jobs.back();std::vector<uint8_t> raw;if(!storage_->raw(r.id,raw))return;size_t len=0;std::vector<unsigned char> b64(4*((raw.size()+2)/3)+1);if(mbedtls_base64_encode(b64.data(),b64.size(),&len,raw.data(),raw.size()))return;JsonDocument j;j["eventId"]=r.id;j["timestamp"]=r.timestamp;j["deviceId"]=String((uint32_t)ESP.getEfuseMac(),HEX);j["sourceIp"]=r.ip;j["sourcePort"]=r.port;j["bytes"]=r.bytes;j["format"]=r.format;j["parsedOrder"]=r.text;j["rawBase64"]=String((char*)b64.data(),len);j["token"]=token;String body;serializeJson(j,body);WiFiClientSecure c;c.setCACert(ca.c_str());HTTPClient http;http.setTimeout(5000);if(!http.begin(c,endpoint)){retry_=millis()+delay_;return;}http.addHeader("Content-Type","application/json");int code=http.POST(body);String answer=http.getString();http.end();JsonDocument ack;if(code==200&&!deserializeJson(ack,answer)&&ack["ok"].as<bool>()&&ack["eventId"].as<String>()==r.id){storage_->acknowledge(r.id);delay_=1000;}else delay_=min<uint32_t>(delay_*2,300000u);retry_=millis()+delay_;}
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
#ifdef BOARD_LCDWIKI_ES3C28P
 adapter_.begin();
#else
 Serial.println("LCD adapter disabled: select LCDWIKI profile for candidate hardware");
#endif
}
void DisplayManager::tick(const Diagnostics& d,CaptureStorage& s,const String& listener){
#ifdef BOARD_LCDWIKI_ES3C28P
 uint32_t now=millis();if(now-lastDraw_<500)return;lastDraw_=now;
#ifdef LCDWIKI_DIAGNOSTIC
 adapter_.clear(0x001f);adapter_.text(8,8,"LCDWIKI DIAGNOSTIC",0xffff);adapter_.text(8,40,"Touch: tap screen",0xffff);
#else
 if(now-lastRefresh_>=2000){lastRefresh_=now;pendingCache_=s.pending();auto records=s.history(4);std::vector<UiEvent> events;for(const auto&r:records)events.push_back({r.id,r.ip,r.format,r.bytes,r.timestamp,r.incomplete,false});model_.setEvents(events);}
 if(model_.selectedId!=loadedId_){loadedId_=model_.selectedId;std::vector<uint8_t> raw;if(s.raw(loadedId_,raw))model_.setRaw(raw);else model_.setRaw({});}
 String wifiState=WiFi.status()==WL_CONNECTED?"CONNECTED":WiFi.getMode()==WIFI_AP?"SETUP AP":"CONNECTING";
 model_.draw(adapter_,WiFi.SSID(),WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():WiFi.softAPIP().toString(),wifiState,listener,pendingCache_);
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
  if(model_)model_->onTouch(x,y);
#endif
 }
 pressed_=down;
#endif
}
