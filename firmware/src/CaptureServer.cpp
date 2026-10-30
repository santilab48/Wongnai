#include "CaptureServer.h"
#include "PrintParser.h"
bool CaptureServer::begin(uint16_t port,CaptureStorage* storage){
 if(!storage||server_){Serial.println("[TCP] ERROR invalid storage or listener already initialized");return false;}
 storage_=storage;server_=new WiFiServer(port);server_->begin();
 if(!*server_){Serial.printf("[TCP] ERROR listen failed port=%u\n",port);delete server_;server_=nullptr;return false;}
 server_->setNoDelay(true);Serial.printf("[TCP] listening on port %u\n",port);return true;
}
void CaptureServer::finish(ClientState& c,bool incomplete){
 if(!c.active)return;
 if(c.capture.finish(incomplete)){
  CaptureRecord r;
  r.id=String((uint32_t)ESP.getEfuseMac(),HEX)+"-"+String(c.last,HEX)+"-"+String(c.sequence,HEX);
  r.ip=c.socket.remoteIP().toString();r.port=c.socket.remotePort();r.timestamp=time(nullptr);
  const auto& bytes=c.capture.bytes();
  auto p=PrintParser::parse(bytes.data(),bytes.size());
  r.format=p.format;r.text=p.text.substring(0,512);r.incomplete=c.capture.incomplete();
  Serial.printf("[TCP] finalized id=%s bytes=%u incomplete=%s\n",r.id.c_str(),(unsigned)bytes.size(),r.incomplete?"yes":"no");
  if(storage_->append(r,bytes.data(),bytes.size()))Serial.printf("[ARCHIVE] saved id=%s bytes=%u\n",r.id.c_str(),(unsigned)bytes.size());
  else{errors_++;Serial.printf("[TCP] ERROR capture received id=%s bytes=%u but archive save failed; see [ARCHIVE] diagnostic\n",r.id.c_str(),(unsigned)bytes.size());}
 }else if(!c.capture.bytes().empty())Serial.println("[TCP] stream was already finalized");
 else Serial.printf("[TCP] client closed without payload sequence=%lu\n",(unsigned long)c.sequence);
 c.socket.stop();c.capture.reset();c.active=false;
}
void CaptureServer::tick(){
 if(!server_)return;
 if(!pollLogged_){Serial.println("[TCP] poll active");pollLogged_=true;}
 // Accept at most one new client per iteration; a full pool closes the connection.
 WiFiClient incoming=server_->accept();
 if(incoming){bool assigned=false;for(auto& c:clients_)if(!c.active){c.socket=incoming;c.active=true;c.last=millis();c.sequence=++accepted_;c.capture.reset();Serial.printf("[TCP] accepted sequence=%lu client=%s:%u\n",(unsigned long)c.sequence,c.socket.remoteIP().toString().c_str(),c.socket.remotePort());assigned=true;break;}if(!assigned){incoming.stop();errors_++;Serial.println("[TCP] rejected client: client pool full");}}
 for(auto& c:clients_){
  if(!c.active)continue;
  uint8_t chunk[1024];size_t budget=4096;
  while(c.socket.available()&&budget){int n=c.socket.read(chunk,min(sizeof chunk,budget));if(n<=0){Serial.printf("[TCP] ERROR read failed sequence=%lu result=%d\n",(unsigned long)c.sequence,n);break;}budget-=n;c.last=millis();if(!c.capture.append(chunk,n)){errors_++;Serial.printf("[TCP] ERROR stream limit exceeded sequence=%lu; closing client\n",(unsigned long)c.sequence);c.socket.stop();c.capture.reset();c.active=false;break;}Serial.printf("[TCP] received sequence=%lu chunk=%d total=%u\n",(unsigned long)c.sequence,n,(unsigned)c.capture.bytes().size());}
  if(c.active&&!c.socket.connected()&&!c.socket.available())finish(c,false);
  else if(c.active&&millis()-c.last>5000)finish(c,true);
 }
}
