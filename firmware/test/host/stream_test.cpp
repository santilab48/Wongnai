#include "CaptureSession.h"
#include "StreamAssembler.h"
#include <cassert>
#include <vector>
int main(){
 StreamAssembler s(65536);std::vector<uint8_t> chunk(1024,0xff);for(int i=0;i<64;i++)assert(s.append(chunk.data(),chunk.size()));assert(s.bytes().size()==65536);assert(!s.append(chunk.data(),1));s.clear();assert(s.bytes().empty());const uint8_t binary[]={0,27,255};assert(s.append(binary,3));assert(s.bytes()[2]==255);
 const uint8_t order[]="TEST ORDER 003";CaptureSession session(32768);assert(session.append(order,sizeof(order)-1));assert(session.bytes().size()==14);assert(session.finish(false));assert(!session.incomplete());assert(session.bytes().size()==14);assert(!session.finish(false));
 session.reset();assert(session.append(order,sizeof(order)-1));assert(session.finish(true));assert(session.incomplete());
}
