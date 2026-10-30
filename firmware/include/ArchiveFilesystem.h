#pragma once

#include <stdint.h>

namespace ArchiveFilesystem {
constexpr const char* MountPath="/littlefs";
constexpr const char* PartitionLabel="spiffs";

template<typename Filesystem>
bool mount(Filesystem& filesystem){
 return filesystem.begin(false,MountPath,10,PartitionLabel);
}

enum class DirectoryStatus {Created,AlreadyPresent,CreateFailed,OpenFailed,NotDirectory};

template<typename Filesystem>
DirectoryStatus ensureDirectory(Filesystem& filesystem,const char* path){
 const bool existed=filesystem.exists(path);
 if(!existed&&!filesystem.mkdir(path))return DirectoryStatus::CreateFailed;
 auto directory=filesystem.open(path,"r");
 if(!directory)return DirectoryStatus::OpenFailed;
 if(!directory.isDirectory())return DirectoryStatus::NotDirectory;
 return existed?DirectoryStatus::AlreadyPresent:DirectoryStatus::Created;
}
}
