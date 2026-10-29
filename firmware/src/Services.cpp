#include "Services.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h>
void WiFiManager::begin(){prefs_.begin("wifi",false);String s=prefs_.getString("ssid","");if(s.length()){WiFi.mode(WIFI_STA);WiFi.begin(s.c_str(),prefs_.getString("password","").c_str());}else{WiFi.mode(WIFI_AP);WiFi.softAP("Wongnai-Setup");}}
void WiFiManager::tick(){if(WiFi.getMode()==WIFI_STA&&WiFi.status()!=WL_CONNECTED&&millis()-retry_>10000){retry_=millis();WiFi.reconnect();}}
String WiFiManager::ssid()const{return WiFi.SSID();}
bool WiFiManager::configure(const String& s,const String& p){if(!s.length())return false;prefs_.putString("ssid",s);prefs_.putString("password",p);WiFi.mode(WIFI_STA);WiFi.begin(s.c_str(),p.c_str());return true;}
String Diagnostics::state(bool listening,bool receiving,size_t freeBytes)const{if(freeBytes<70000)return "STORAGE FULL";if(receiving)return "RECEIVING";if(!listening)return "OFFLINE";return "LISTENING";}
void GoogleSheetsUploader::begin(CaptureStorage* s){storage_=s;prefs_.begin("upload",true);}
void GoogleSheetsUploader::tick(){if(WiFi.status()!=WL_CONNECTED||!storage_||millis()<retry_)return;String endpoint=prefs_.getString("url","");String token=prefs_.getString("token","");String ca=prefs_.getString("ca","");if(!endpoint.startsWith("https://")||!token.length()||!ca.length())return;auto jobs=storage_->history(100);if(jobs.empty())return;auto r=jobs.back();std::vector<uint8_t> raw;if(!storage_->raw(r.id,raw))return;size_t len=0;std::vector<unsigned char> b64(4*((raw.size()+2)/3)+1);if(mbedtls_base64_encode(b64.data(),b64.size(),&len,raw.data(),raw.size()))return;JsonDocument j;j["eventId"]=r.id;j["timestamp"]=r.timestamp;j["deviceId"]=String((uint32_t)ESP.getEfuseMac(),HEX);j["sourceIp"]=r.ip;j["sourcePort"]=r.port;j["bytes"]=r.bytes;j["format"]=r.format;j["parsedOrder"]=r.text;j["rawBase64"]=String((char*)b64.data(),len);j["token"]=token;String body;serializeJson(j,body);WiFiClientSecure c;c.setCACert(ca.c_str());HTTPClient http;http.setTimeout(5000);if(!http.begin(c,endpoint)){retry_=millis()+delay_;return;}http.addHeader("Content-Type","application/json");int code=http.POST(body);String answer=http.getString();http.end();JsonDocument ack;if(code==200&&!deserializeJson(ack,answer)&&ack["ok"].as<bool>()&&ack["eventId"].as<String>()==r.id){storage_->acknowledge(r.id);delay_=1000;}else delay_=min(delay_*2,300000u);retry_=millis()+delay_;}
void EmailReporter::begin(){prefs_.begin("email",true);}
void EmailReporter::tick(const Diagnostics& d){(void)d;/* Reports are sent by the Apps Script endpoint after acknowledged uploads. */}
void DisplayManager::begin(){Serial.println("LCD adapter disabled: controller and GPIO map unverified");}
void DisplayManager::tick(const Diagnostics& d,const CaptureStorage& s){(void)d;(void)s;}
void TouchInput::begin(){}void TouchInput::tick(){}
