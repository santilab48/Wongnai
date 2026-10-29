#pragma once
#include <Arduino.h>
#include <FS.h>
#include <vector>
struct CaptureRecord {String id,ip,format,text;uint16_t port=0;uint64_t timestamp=0;size_t bytes=0;bool uploaded=false,incomplete=false;};
class CaptureStorage {
public:
 bool begin(); bool append(const CaptureRecord& r,const uint8_t* data,size_t n); bool acknowledge(const String& id); std::vector<CaptureRecord> history(size_t limit=20); bool raw(const String& id,std::vector<uint8_t>& out); size_t pending(); size_t freeBytes(); uint32_t lost() const{return lost_;}
private:
 uint32_t lost_=0; String path(const String& id) const; bool readMeta(const String& id,CaptureRecord& r);
};
