#include "ArchiveFilesystem.h"
#include "ArchiveFormat.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs=std::filesystem;

struct TestDirectory {
 fs::path path;
 explicit TestDirectory(const char* label){
  const auto nonce=std::chrono::steady_clock::now().time_since_epoch().count();
  path=fs::temp_directory_path()/(std::string("wongnai-")+label+"-"+std::to_string(nonce));
  fs::create_directories(path);
 }
 ~TestDirectory(){fs::remove_all(path);}
};

struct TestFile {
 bool directory=false,valid=false;
 explicit operator bool()const{return valid;}
 bool isDirectory()const{return directory;}
};

struct TestFilesystem {
 fs::path root;
 bool mountResult=true,mounted=false,formatOnFail=true;
 bool mkdirCalled=false;
 std::string basePath,partitionLabel,failMkdirPath,failOpenPath;
 TestFilesystem(const fs::path& path,bool result=true):root(path),mountResult(result){}
 bool begin(bool format,const char* base,uint8_t,const char* label){
  formatOnFail=format;basePath=base;partitionLabel=label;mounted=mountResult;return mounted;
 }
 bool exists(const char* path)const{return mounted&&fs::exists(root/(path+1));}
 bool mkdir(const char* path){
  mkdirCalled=true;
  if(!mounted||failMkdirPath==path)return false;
  return fs::create_directory(root/(path+1));
 }
 TestFile open(const char* path,const char*){
  if(!mounted||failOpenPath==path)return {};
  const auto target=root/(path+1);
  return {fs::is_directory(target),fs::exists(target)};
 }
};

static void createArchiveRecord(const fs::path& root,const std::vector<uint8_t>& raw){
 ArchiveFormat::Header header{ArchiveFormat::Magic,static_cast<uint32_t>(raw.size()),ArchiveFormat::crc32(raw.data(),raw.size()),0};
 std::ofstream record(root/"a"/"capture.rec",std::ios::binary);
 record.write(reinterpret_cast<const char*>(&header),sizeof(header));
 record.write(reinterpret_cast<const char*>(raw.data()),static_cast<std::streamsize>(raw.size()));
 assert(record.good());
 std::ofstream marker(root/"q"/"capture.pending",std::ios::binary);
 assert(marker.good());
}

static std::vector<uint8_t> readArchiveRecord(const fs::path& path){
 std::ifstream record(path,std::ios::binary|std::ios::ate);
 assert(record.good());
 const auto fileSize=static_cast<size_t>(record.tellg());
 record.seekg(0);
 ArchiveFormat::Header header{};
 record.read(reinterpret_cast<char*>(&header),sizeof(header));
 std::vector<uint8_t> raw(header.length);
 record.read(reinterpret_cast<char*>(raw.data()),static_cast<std::streamsize>(raw.size()));
 assert(ArchiveFormat::valid(header,fileSize,raw.data()));
 return raw;
}

int main(){
 const std::vector<uint8_t> original={0,0xff,0x1b,'O','R','D','E','R',0};

 TestDirectory failedMountRoot("failed-mount");
 TestFilesystem failedMount(failedMountRoot.path,false);
 assert(!ArchiveFilesystem::mount(failedMount));
 assert(!failedMount.formatOnFail);
 assert(failedMount.basePath=="/littlefs"&&failedMount.partitionLabel=="spiffs");
 assert(!failedMount.mkdirCalled&&!fs::exists(failedMountRoot.path/"a"));

 TestDirectory freshRoot("fresh");
 TestFilesystem firstBoot(freshRoot.path);
 assert(ArchiveFilesystem::mount(firstBoot));
 assert(ArchiveFilesystem::ensureDirectory(firstBoot,"/a")==ArchiveFilesystem::DirectoryStatus::Created);
 assert(ArchiveFilesystem::ensureDirectory(firstBoot,"/q")==ArchiveFilesystem::DirectoryStatus::Created);
 assert(fs::is_directory(freshRoot.path/"a")&&fs::is_directory(freshRoot.path/"q"));
 createArchiveRecord(freshRoot.path,original);
 assert(readArchiveRecord(freshRoot.path/"a"/"capture.rec")==original);

 TestFilesystem afterRestart(freshRoot.path);
 assert(ArchiveFilesystem::mount(afterRestart));
 assert(ArchiveFilesystem::ensureDirectory(afterRestart,"/a")==ArchiveFilesystem::DirectoryStatus::AlreadyPresent);
 assert(ArchiveFilesystem::ensureDirectory(afterRestart,"/q")==ArchiveFilesystem::DirectoryStatus::AlreadyPresent);
 assert(fs::exists(freshRoot.path/"q"/"capture.pending"));
 assert(readArchiveRecord(freshRoot.path/"a"/"capture.rec")==original);

 TestDirectory failedDirectoryRoot("directory-fail");
 TestFilesystem failedDirectory(failedDirectoryRoot.path);
 assert(ArchiveFilesystem::mount(failedDirectory));
 failedDirectory.failMkdirPath="/a";
 assert(ArchiveFilesystem::ensureDirectory(failedDirectory,"/a")==ArchiveFilesystem::DirectoryStatus::CreateFailed);
 assert(!fs::exists(failedDirectoryRoot.path/"a"));

 failedDirectory.failMkdirPath.clear();
 assert(ArchiveFilesystem::ensureDirectory(failedDirectory,"/a")==ArchiveFilesystem::DirectoryStatus::Created);
 failedDirectory.failOpenPath="/a";
 assert(ArchiveFilesystem::ensureDirectory(failedDirectory,"/a")==ArchiveFilesystem::DirectoryStatus::OpenFailed);
}
