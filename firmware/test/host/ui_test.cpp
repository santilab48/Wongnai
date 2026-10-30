#include "UiModel.h"
#include "StreamAssembler.h"
#include <cassert>
struct FakeDisplay:DisplayAdapter{
 String output;int fullClears=0,regionClears=0,textCalls=0,buttonCalls=0;std::vector<String> operations;
 void clear(uint16_t)override{fullClears++;operations.push_back("clear");output.clear();}
 void clearRect(int,int,int,int,uint16_t)override{regionClears++;operations.push_back("region");}
 void text(int,int,const String&s,uint16_t,uint8_t=2)override{ textCalls++;operations.push_back("text");output+=s;output+=' ';}
 void button(int,int,int,int,const String&s)override{buttonCalls++;operations.push_back("button");output+=s;output+=' ';}
};
int main(){
 UiModel emptyUi;
 FakeDisplay emptyDisplay;
 emptyUi.draw(emptyDisplay,"wifi","192.0.2.1","CONNECTED","RECEIVING",0,0,0);
 assert(emptyDisplay.output.find("Receiving TCP data")!=String::npos);
 assert(emptyDisplay.output.find("Flash unavailable")!=String::npos);
 emptyDisplay.output.clear();
 emptyUi.draw(emptyDisplay,"wifi","192.0.2.1","CONNECTED","LISTENING",0,0,0);
 assert(emptyDisplay.output.find("Waiting for TCP data")!=String::npos);
 assert(emptyDisplay.output.find("STORAGE OFFLINE") == String::npos);
 UiModel ui;
 ui.setEvents({{"old","1.1.1.1","TEXT",2,100,false,false},{"new","2.2.2.2","ESC",3,200,true,false}});
 assert(ui.recent[0].id=="new");
 FakeDisplay d;
 ui.draw(d,"wifi","192.0.2.1","CONNECTED","LISTENING",2,100000,1000000);
 assert(d.output.find("2.2.2.2")<d.output.find("1.1.1.1"));
 assert(d.output.find("RECEIVED INCOMPLETE")!=String::npos);
 assert(d.fullClears==1);
 const int initialTextCalls=d.textCalls,initialButtonCalls=d.buttonCalls,initialRegionClears=d.regionClears;
 ui.draw(d,"wifi","192.0.2.1","CONNECTED","LISTENING",2,100000,1000000);
 assert(d.fullClears==1&&d.textCalls==initialTextCalls&&d.buttonCalls==initialButtonCalls&&d.regionClears==initialRegionClears);
 const int beforeChange=d.textCalls,beforeRegions=d.regionClears;
 d.operations.clear();
 ui.draw(d,"wifi","192.0.2.1","OFFLINE","LISTENING",2,100000,1000000);
 assert(d.textCalls==beforeChange+1&&d.regionClears==beforeRegions+1&&d.fullClears==1);
 assert(d.operations.size()==2&&d.operations[0]=="region"&&d.operations[1]=="text");
 ui.setEvents({{"old","1.1.1.1","TEXT",2,100,false,false},{"newer","3.3.3.3","ESC",4,300,false,false},{"new","2.2.2.2","ESC",3,200,true,false}});
 d.output.clear();
 ui.draw(d,"wifi","192.0.2.1","OFFLINE","LISTENING",3,100000,1000000);
 assert(d.output.find("3.3.3.3")<d.output.find("2.2.2.2")&&d.output.find("2.2.2.2")<d.output.find("1.1.1.1"));
 ui.onTouch(130,10);
 assert(ui.page==UiPage::History);
 ui.onTouch(130,10);
 assert(ui.page==UiPage::LiveCapture);
 uint8_t bytes[]={0,27,255};
 StreamAssembler stream(32768);
 assert(stream.append(bytes,3));
 ui.onTouch(20,110);
 assert(ui.page==UiPage::RawHex&&ui.selectedId=="new");
 ui.setRaw(stream.bytes());
 d.operations.clear();
 ui.draw(d,"","","","",0,100000,1000000);
 assert(d.output.find("00 1b ff")!=String::npos);
 assert(d.fullClears==2);
 ui.onTouch(170,40);
 ui.draw(d,"","","","",0,100000,1000000);
 assert(ui.page==UiPage::RawText);
 assert(ui.selectedRaw==stream.bytes());
 ui.onTouch(20,40);
 assert(ui.page==UiPage::History);
 ui.onTouch(200,10);
 assert(ui.page==UiPage::WiFiSettings);
 ui.onTouch(8,143);
 assert(ui.wifiSsid=="A");
 ui.onTouch(140,110);
 assert(ui.connectRequested);
}
