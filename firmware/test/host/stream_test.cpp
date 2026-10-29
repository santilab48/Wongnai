#include "StreamAssembler.h"
#include <cassert>
#include <vector>
int main(){StreamAssembler s(65536);std::vector<uint8_t> chunk(1024,0xff);for(int i=0;i<64;i++)assert(s.append(chunk.data(),chunk.size()));assert(s.bytes().size()==65536);assert(!s.append(chunk.data(),1));s.clear();assert(s.bytes().empty());const uint8_t binary[]={0,27,255};assert(s.append(binary,3));assert(s.bytes()[2]==255);}
