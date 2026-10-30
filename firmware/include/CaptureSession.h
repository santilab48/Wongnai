#pragma once
#include "StreamAssembler.h"

class CaptureSession {
public:
 explicit CaptureSession(size_t limit=65536):stream_(limit){}
 bool append(const uint8_t* data,size_t n){return !finalized_&&stream_.append(data,n);}
 bool finish(bool incomplete){
  if(finalized_)return false;
  finalized_=true;
  incomplete_=incomplete;
  return !stream_.bytes().empty();
 }
 const std::vector<uint8_t>& bytes()const{return stream_.bytes();}
 bool incomplete()const{return incomplete_;}
 void reset(){stream_.clear();finalized_=false;incomplete_=false;}
private:
 StreamAssembler stream_;
 bool finalized_=false,incomplete_=false;
};
