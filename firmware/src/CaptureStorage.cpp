#include "CaptureStorage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_partition.h>
#include <algorithm>
#include <cstring>
#include "ArchiveFormat.h"
#include "ArchiveFilesystem.h"
using ArchiveFormat::Header;
using ArchiveFormat::crc32;
static constexpr uint32_t MAGIC=ArchiveFormat::Magic;
static constexpr size_t RESERVE=70000;
String CaptureStorage::path(const String& id) const{return "/a/"+id+".rec";}
String CaptureStorage::queuePath(const String& id) const{return "/q/"+id+".pending";}
static bool validId(const String& id){if(!id.length()||id.length()>80)return false;for(size_t i=0;i<id.length();i++){char c=id[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'))return false;}return true;}
void CaptureStorage::logDirectoryError(const char* path,const char* reason){
 size_t index=strcmp(path,"/a")==0?0:1;
 if(directoryErrorsLogged_[index])return;
 directoryErrorsLogged_[index]=true;
 Serial.printf("[ARCHIVE] directory error path=%s reason=%s\n",path,reason);
}
File CaptureStorage::openDirectory(const char* path){
 if(!available())return File();
 File directory=LittleFS.open(path,"r");
 if(!directory){directoriesReady_=false;logDirectoryError(path,"open failed");}
 else if(!directory.isDirectory()){directory.close();directoriesReady_=false;logDirectoryError(path,"path is not a directory");return File();}
 return directory;
}
bool CaptureStorage::ensureDirectory(const char* path){
 const auto status=ArchiveFilesystem::ensureDirectory(LittleFS,path);
 if(status==ArchiveFilesystem::DirectoryStatus::Created)Serial.printf("[ARCHIVE] directory created path=%s\n",path);
 else if(status==ArchiveFilesystem::DirectoryStatus::CreateFailed)logDirectoryError(path,"creation failed");
 else if(status==ArchiveFilesystem::DirectoryStatus::OpenFailed)logDirectoryError(path,"open failed after create check");
 else if(status==ArchiveFilesystem::DirectoryStatus::NotDirectory)logDirectoryError(path,"path is not a directory");
 return status==ArchiveFilesystem::DirectoryStatus::Created||status==ArchiveFilesystem::DirectoryStatus::AlreadyPresent;
}
bool CaptureStorage::begin(){
 const esp_partition_t* partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_SPIFFS,"spiffs");
 if(!partition){Serial.println("[ARCHIVE] partition missing label=spiffs subtype=spiffs");return false;}
 Serial.printf("[ARCHIVE] partition label=%s offset=0x%06lx size=0x%06lx (%lu bytes)\n",partition->label,(unsigned long)partition->address,(unsigned long)partition->size,(unsigned long)partition->size);
 if(!ArchiveFilesystem::mount(LittleFS)){Serial.println("[ARCHIVE] mount failed partition=spiffs format=disabled");return false;}
 mounted_=true;
 if(!ensureDirectory("/a")||!ensureDirectory("/q"))return false;
 directoriesReady_=true;
 File d=openDirectory("/a");if(!d)return false;
 File f=d.openNextFile();while(f){String name=f.name();f=d.openNextFile();if(name.endsWith(".tmp")){String id=name.substring(name.lastIndexOf('/')+1,name.length()-8);if(validId(id)&&LittleFS.exists(queuePath(id))&&!LittleFS.exists(path(id))){String target=path(id);if(LittleFS.rename(name,target)){CaptureRecord recovered;if(!readMeta(id,recovered)){LittleFS.rename(target,name);lost_++;}}else lost_++;}else if(!LittleFS.exists(queuePath(id)))LittleFS.remove(name);}}
 d=openDirectory("/q");if(!d)return false;
 f=d.openNextFile();while(f){String name=f.name();f=d.openNextFile();if(name.endsWith(".tmp")){LittleFS.remove(name);continue;}if(name.endsWith(".pending")){String id=name.substring(name.lastIndexOf('/')+1,name.length()-8);if(!LittleFS.exists(path(id))&&!LittleFS.exists(path(id)+".tmp")){LittleFS.remove(name);lost_++;}continue;}if(name.endsWith(".rec")){String id=name.substring(name.lastIndexOf('/')+1,name.length()-4);String target=path(id);if(validId(id)&&!LittleFS.exists(target)){if(!LittleFS.rename(name,target))continue;}else if(validId(id)){CaptureRecord r;if(!readMeta(id,r))continue;LittleFS.remove(name);}if(validId(id)&&LittleFS.exists(target)&&!LittleFS.exists(queuePath(id))){File marker=LittleFS.open(queuePath(id),"w");if(marker)marker.close();else Serial.printf("[ARCHIVE] recovery marker creation failed id=%s\n",id.c_str());}}}
 Serial.printf("[ARCHIVE] mounted partition=spiffs capacity=%lu bytes; lost records=%lu\n",(unsigned long)LittleFS.totalBytes(),(unsigned long)lost_);return true;
}
size_t CaptureStorage::freeBytes(){if(!available())return 0;size_t total=LittleFS.totalBytes(),used=LittleFS.usedBytes();return used<=total?total-used:0;}
size_t CaptureStorage::capacity(){return available()?LittleFS.totalBytes():0;}
bool CaptureStorage::append(const CaptureRecord& r,const uint8_t* data,size_t n){
 auto failed=[&](const char* reason){lost_++;Serial.printf("[ARCHIVE] append rejected id=%s reason=%s\n",r.id.c_str(),reason);return false;};
 if(!available())return failed("archive filesystem unavailable");
 if(!validId(r.id)||!data||!n||n>32768)return failed("invalid record or payload");
 if(LittleFS.exists(path(r.id))||LittleFS.exists(queuePath(r.id)))return failed("duplicate id");
 JsonDocument j;j["id"]=r.id;j["ip"]=r.ip;j["port"]=r.port;j["timestamp"]=r.timestamp;j["format"]=r.format;j["text"]=r.text;j["incomplete"]=r.incomplete;String meta;serializeJson(j,meta);
 if(!ArchiveFormat::fits(freeBytes(),n,meta.length(),RESERVE))return failed("insufficient archive space");
 Header h{MAGIC,(uint32_t)n,crc32(data,n),(uint32_t)meta.length()};String tmp=path(r.id)+".tmp";File f=LittleFS.open(tmp,"w");if(!f)return failed("temporary record open failed");
 bool ok=f.write((uint8_t*)&h,sizeof h)==sizeof h&&f.write((uint8_t*)meta.c_str(),meta.length())==meta.length()&&f.write(data,n)==n;f.flush();f.close();if(!ok){LittleFS.remove(tmp);return failed("record write failed");}
 File marker=LittleFS.open(queuePath(r.id),"w");if(!marker){LittleFS.remove(tmp);return failed("queue marker open failed");}
 marker.close();if(!LittleFS.rename(tmp,path(r.id))){LittleFS.remove(tmp);LittleFS.remove(queuePath(r.id));return failed("archive commit failed");}
 return true;
}
bool CaptureStorage::readMeta(const String& id,CaptureRecord& r){if(!available()||!validId(id))return false;File f=LittleFS.open(path(id),"r");Header h;if(!f||f.readBytes((char*)&h,sizeof h)!=sizeof h||h.magic!=MAGIC||!h.length||h.length>32768||h.metaLength>2048||f.size()!=sizeof h+h.metaLength+h.length)return false;String m;for(uint32_t i=0;i<h.metaLength;i++){int c=f.read();if(c<0)return false;m+=(char)c;}std::vector<uint8_t> b(h.length);if(f.read(b.data(),b.size())!=b.size()||crc32(b.data(),b.size())!=h.crc)return false;JsonDocument j;if(deserializeJson(j,m)||j["id"].as<String>()!=id)return false;r.id=id;r.ip=j["ip"].as<String>();r.port=j["port"]|0;r.timestamp=j["timestamp"].as<uint64_t>();r.format=j["format"].as<String>();r.text=j["text"].as<String>();r.bytes=h.length;r.incomplete=j["incomplete"]|false;r.uploaded=!LittleFS.exists(queuePath(id));return true;}
std::vector<CaptureRecord> CaptureStorage::list(const char* dir,const char* suffix,size_t limit){std::vector<CaptureRecord> v;if(!available())return v;File d=openDirectory(dir);if(!d)return v;File f=d.openNextFile();while(f){String name=f.name();f=d.openNextFile();if(!name.endsWith(suffix))continue;String id=name.substring(name.lastIndexOf('/')+1,name.length()-strlen(suffix));CaptureRecord r;if(readMeta(id,r))v.push_back(r);else Serial.printf("[ARCHIVE] ERROR invalid record path=%s\n",name.c_str());}std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.timestamp==b.timestamp?a.id>b.id:a.timestamp>b.timestamp;});if(v.size()>limit)v.resize(limit);return v;}
std::vector<CaptureRecord> CaptureStorage::history(size_t limit){return list("/a",".rec",limit);}
std::vector<CaptureRecord> CaptureStorage::uploadQueue(size_t limit){return list("/q",".pending",limit);}
bool CaptureStorage::raw(const String& id,std::vector<uint8_t>& out){CaptureRecord r;if(!readMeta(id,r))return false;File f=LittleFS.open(path(id),"r");Header h;if(f.readBytes((char*)&h,sizeof h)!=sizeof h||!f.seek(sizeof h+h.metaLength))return false;std::vector<uint8_t> bytes(h.length);if(f.read(bytes.data(),bytes.size())!=bytes.size()||crc32(bytes.data(),bytes.size())!=h.crc)return false;out.swap(bytes);return true;}
bool CaptureStorage::acknowledge(const String& id){CaptureRecord r;if(!readMeta(id,r)||!LittleFS.exists(queuePath(id)))return false;return LittleFS.remove(queuePath(id));}
size_t CaptureStorage::pending(){if(!available())return 0;File d=openDirectory("/q");if(!d)return 0;File f=d.openNextFile();size_t count=0;while(f){String name=f.name();f=d.openNextFile();if(name.endsWith(".pending"))count++;}return count;}
