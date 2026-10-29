#include "CaptureServer.h"
#include "PrintParser.h"
bool CaptureServer::begin(uint16_t port,CaptureStorage* storage){if(!storage||server_)return false;storage_=storage;server_=new WiFiServer(port);server_->begin();server_->setNoDelay(true);return true;}
void CaptureServer::finish(ClientState& c,bool incomplete){
 if(!c.active)return;
 if(!c.bytes.bytes().empty()){
  CaptureRecord r;
  r.id=String((uint32_t)ESP.getEfuseMac(),HEX)+"-"+String(c.last,HEX)+"-"+String(c.sequence,HEX);
  r.ip=c.socket.remoteIP().toString();r.port=c.socket.remotePort();r.timestamp=time(nullptr);
  auto p=PrintParser::parse(c.bytes.bytes().data(),c.bytes.bytes().size());
  r.format=p.format;r.text=p.text.substring(0,512);r.incomplete=incomplete;
  if(!storage_->append(r,c.bytes.bytes().data(),c.bytes.bytes().size()))errors_++;
 }
 c.socket.stop();c.bytes.clear();c.active=false;
}
void CaptureServer::tick(){
 if(!server_)return;
 // Accept at most one new client per iteration; a full pool closes the connection.
 WiFiClient incoming=server_->available();
 if(incoming){bool assigned=false;for(auto& c:clients_)if(!c.active){c.socket=incoming;c.active=true;c.last=millis();c.sequence=++accepted_;c.bytes.clear();assigned=true;break;}if(!assigned){incoming.stop();errors_++;}}
 for(auto& c:clients_){
  if(!c.active)continue;
  uint8_t chunk[1024];size_t budget=4096;
  while(c.socket.available()&&budget){int n=c.socket.read(chunk,min(sizeof chunk,budget));if(n<=0)break;budget-=n;c.last=millis();if(!c.bytes.append(chunk,n)){errors_++;c.socket.stop();c.bytes.clear();c.active=false;break;}}
  if(c.active&&!c.socket.connected()&&!c.socket.available())finish(c,false);
  else if(c.active&&millis()-c.last>5000)finish(c,true);
 }
}
