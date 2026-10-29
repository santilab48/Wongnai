#include "UiModel.h"
void UiModel::draw(DisplayAdapter& d,const String& ssid,const String& ip,const String& state,size_t pending){
 d.clear(0xffff);d.text(8,8,"Wongnai Capture V1",0x0000);
 if(page==UiPage::Dashboard){d.text(8,42,"Wi-Fi: "+ssid,0x0000);d.text(8,64,"IP: "+ip,0x0000);d.text(8,86,state,0x0000);d.text(8,108,"Pending: "+String(pending),0x0000);d.button(8,150,224,42,"Live Capture");d.button(8,200,224,42,"History");d.button(8,250,224,42,"Settings");}
 else if(page==UiPage::LiveCapture||page==UiPage::History){d.text(8,40,page==UiPage::LiveCapture?"Live Capture":"History",0x0000);for(size_t i=0;i<recent.size()&&i<8;i++)d.text(8,65+i*26,recent[i].ip+" "+String(recent[i].bytes)+"B "+recent[i].format,0x0000);}
 else if(page==UiPage::RawText||page==UiPage::RawHex){d.text(8,40,page==UiPage::RawText?"Raw Text":"Raw Hex",0x0000);d.text(8,66,"Select an event from History",0x0000);}
 else d.text(8,40,"Hardware input pending verification",0x0000);
}
void UiModel::onTouch(int x,int y){if(x<8||x>232)return;if(page==UiPage::Dashboard){if(y>=150&&y<192)page=UiPage::LiveCapture;else if(y>=200&&y<242)page=UiPage::History;else if(y>=250&&y<292)page=UiPage::Settings;}}
