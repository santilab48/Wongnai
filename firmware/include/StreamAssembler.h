#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vector>
// Bounded, binary-safe TCP stream accumulator. One connection is one event.
class StreamAssembler {
public:
 explicit StreamAssembler(size_t limit=65536):limit_(limit){}
 bool append(const uint8_t* data,size_t n){if(n>limit_-bytes_.size())return false;bytes_.insert(bytes_.end(),data,data+n);return true;}
 const std::vector<uint8_t>& bytes() const{return bytes_;}
 void clear(){bytes_.clear();}
private:size_t limit_;std::vector<uint8_t> bytes_;
};
