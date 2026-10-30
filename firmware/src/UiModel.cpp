#include "UiModel.h"
#include <algorithm>
#include <time.h>

void UiModel::setEvents(const std::vector<UiEvent>& events){
 recent=events;
 std::sort(recent.begin(),recent.end(),[](const UiEvent&a,const UiEvent&b){return a.timestamp==b.timestamp?a.id>b.id:a.timestamp>b.timestamp;});
 if(recent.size()>20)recent.resize(20);
}
static String stamp(uint64_t seconds){if(seconds<1700000000)return "time pending";time_t t=seconds;struct tm tm;gmtime_r(&t,&tm);char buf[24];strftime(buf,sizeof(buf),"%m-%d %H:%M:%S",&tm);return String(buf)+" UTC";}
static String fitText(const String& value,int width,uint8_t size){
 const size_t maxChars=width/(6*size);
 return value.length()>maxChars?value.substring(0,maxChars):value;
}
void UiModel::drawText(DisplayAdapter& d,size_t field,int x,int y,int w,int h,const String& value,uint16_t color,uint8_t size){
 if(renderedFields_.size()<=field)renderedFields_.resize(field+1);
 auto& rendered=renderedFields_[field];
 const String fitted=fitText(value,w,size);
 if(rendered.valid&&rendered.value==fitted)return;
 d.clearRect(x,y,w,h,0xffff);
 if(fitted.length())d.text(x,y,fitted,color,size);
 rendered.value=fitted;
 rendered.valid=true;
}
void UiModel::drawButton(DisplayAdapter& d,size_t field,int x,int y,int w,int h,const String& label){
 if(renderedFields_.size()<=field)renderedFields_.resize(field+1);
 auto& rendered=renderedFields_[field];
 if(rendered.valid&&rendered.value==label)return;
 d.button(x,y,w,h,label);
 rendered.value=label;
 rendered.valid=true;
}
void UiModel::draw(DisplayAdapter& d,const String& ssid,const String& ip,const String& wifiState,const String& listenerState,size_t pending,size_t freeBytes,size_t capacity){
 if(!hasRenderedPage_||renderedPage_!=page){
  d.clear(0xffff);
  renderedFields_.clear();
  renderedPage_=page;
  hasRenderedPage_=true;
 }
 drawText(d,0,5,5,100,18,page==UiPage::History?"HISTORY":"LIVE",0x0000);
 if(page==UiPage::LiveCapture||page==UiPage::History){
  drawButton(d,1,115,5,55,25,page==UiPage::History?"Live":"Hst");
  drawButton(d,2,175,5,60,25,"Wi");
  drawText(d,3,5,27,230,8,"WiFi "+wifiState+" "+ssid,0x0000,1);
  drawText(d,4,5,48,230,16,"IP "+ip,0x0000);
  drawText(d,5,5,69,230,16,"TCP "+listenerState+" Q:"+String(pending),0x0000);
  const size_t used=capacity>=freeBytes?capacity-freeBytes:0;
  drawText(d,6,5,87,230,8,capacity==0?"Flash unavailable":"Flash "+String(used/1024)+"/"+String(capacity/1024)+"K"+(freeBytes<70000?" FULL":""),0x0000,1);
  bool hasEvents=false;
  for(size_t i=0;i<4;i++){
   const size_t field=7+i*3;
   const int y=106+static_cast<int>(i)*50;
   if(i<recent.size()){
    const auto&e=recent[i];
    hasEvents=true;
    drawText(d,field,5,y,230,8,stamp(e.timestamp),0x0000,1);
    drawText(d,field+1,5,y+10,230,8,e.ip+" "+String(e.bytes)+"B "+e.format,0x0000,1);
    drawText(d,field+2,5,y+20,230,8,(e.simulated?"SIMULATED ":"RECEIVED ")+String(e.incomplete?"INCOMPLETE":e.uploaded?"UPLOADED":"PENDING"),0x0000,1);
   }else{
    const String emptyMessage=!hasEvents&&i==0?(listenerState=="RECEIVING"?"Receiving TCP data":"Waiting for TCP data"):"";
    drawText(d,field,5,y,230,8,emptyMessage,0x0000,1);
    drawText(d,field+1,5,y+10,230,8,"",0x0000,1);
    drawText(d,field+2,5,y+20,230,8,"",0x0000,1);
   }
  }
 }else if(page==UiPage::RawHex||page==UiPage::RawText){
  drawButton(d,1,5,30,70,30,"Back");
  drawButton(d,2,85,30,65,30,"HEX");
  drawButton(d,3,160,30,75,30,"TEXT");
  drawText(d,4,5,70,230,16,selectedId.substring(0,22),0x0000);
  const size_t n=std::min(selectedRaw.size(),(size_t)96);
  const size_t rows=(n+7)/8;
  for(size_t row=0;row<12;row++){
   String line;
   if(row<rows){
    for(size_t j=row*8;j<n&&j<row*8+8;j++){
     uint8_t b=selectedRaw[j];
     if(page==UiPage::RawHex){if(b<16)line+="0";line+=String(b,HEX);line+=" ";}
     else line+=(b>=32&&b<=126)?(char)b:'.';
    }
   }
   drawText(d,5+row,5,96+static_cast<int>(row)*11,230,8,line,0x0000,1);
  }
  drawText(d,21,5,302,230,8,String(selectedRaw.size())+" bytes (first 96)",0x0000,1);
 }else{
  drawText(d,3,5,32,230,16,"WiFi setup",0x0000);
  drawText(d,4,5,55,230,16,"SSID: "+wifiSsid,0x0000);
  drawText(d,5,5,76,230,16,"Password: "+String(wifiPassword.length(),'*'),0x0000);
  drawButton(d,6,5,100,110,30,editingPassword?"Password":"SSID");
  drawButton(d,7,125,100,110,30,"Connect");
  const String keys="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for(int i=0;i<36;i++){int x=5+(i%6)*39,y=140+(i/6)*25;drawText(d,8+i,x,y,32,16,String(keys[i]),0x0000,2);}
  drawButton(d,44,5,293,65,25,"Back");
  drawButton(d,45,80,293,70,25,"Delete");
  drawButton(d,46,160,293,75,25,"Clear");
 }
}
void UiModel::onTouch(int x,int y){
 if(x<0||x>=240||y<0||y>=320)return;
 if(page==UiPage::LiveCapture||page==UiPage::History){
  if(y<32){
   if(x>=175){page=UiPage::WiFiSettings;return;}
   if(x>=115){page=page==UiPage::History?UiPage::LiveCapture:UiPage::History;return;}
  }
  if(y>=106){size_t i=(y-106)/50;if(i<recent.size()&&i<4){selectedId=recent[i].id;selectedRaw.clear();page=UiPage::RawHex;}}
 }else if(page==UiPage::RawHex||page==UiPage::RawText){
  if(y>=30&&y<65){if(x<80)page=UiPage::History;else if(x<155)page=UiPage::RawHex;else page=UiPage::RawText;}
 }else if(page==UiPage::WiFiSettings){
  if(y>=100&&y<135){if(x<120)editingPassword=!editingPassword;else connectRequested=true;}
  else if(y>=140&&y<290){int col=(x-5)/39,row=(y-140)/25;if(x>=5&&col<6&&row<6){const String keys="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";(editingPassword?wifiPassword:wifiSsid)+=keys[row*6+col];}}
  else if(y>=293){if(x<75)page=UiPage::LiveCapture;else if(x<155){String&v=editingPassword?wifiPassword:wifiSsid;if(v.length())v.remove(v.length()-1);}else (editingPassword?wifiPassword:wifiSsid)="";}
 }
}
