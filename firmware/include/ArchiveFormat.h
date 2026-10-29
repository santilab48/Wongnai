#pragma once
#include <stddef.h>
#include <stdint.h>
namespace ArchiveFormat {
constexpr uint32_t Magic=0x574f4e47;
constexpr size_t MaxPayload=32768;
constexpr size_t MaxMetadata=2048;
struct Header {uint32_t magic,length,crc,metaLength;};
inline uint32_t crc32(const uint8_t* p,size_t n){uint32_t c=~0u;for(size_t i=0;i<n;i++){c^=p[i];for(int k=0;k<8;k++)c=(c>>1)^((c&1)?0xedb88320u:0);}return ~c;}
inline bool fits(size_t freeBytes,size_t payload,size_t metadata,size_t reserve){return payload>0&&payload<=MaxPayload&&metadata<=MaxMetadata&&freeBytes>=reserve&&payload+metadata+sizeof(Header)<=freeBytes-reserve;}
inline bool valid(const Header& h,size_t fileSize,const uint8_t* payload){return h.magic==Magic&&h.length>0&&h.length<=MaxPayload&&h.metaLength<=MaxMetadata&&fileSize==sizeof(Header)+h.metaLength+h.length&&payload&&crc32(payload,h.length)==h.crc;}
}
