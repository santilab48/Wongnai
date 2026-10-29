#include "PrintParser.h"
#include <stdio.h>
static bool validUtf8(const String& s){const unsigned char* p=(const unsigned char*)s.c_str();size_t n=s.length();for(size_t i=0;i<n;){unsigned char c=p[i++];if(c<0x80)continue;int extra=(c>=0xc2&&c<=0xdf)?1:(c>=0xe0&&c<=0xef)?2:(c>=0xf0&&c<=0xf4)?3:-1;if(extra<0||i+extra>n)return false;for(int j=0;j<extra;j++)if((p[i++]&0xc0)!=0x80)return false;}return true;}
ParsedPrint PrintParser::parse(const uint8_t* p,size_t n){
  ParsedPrint r; String txt; bool esc=false;
  for(size_t i=0;i<n;i++){
    uint8_t c=p[i];
    if(c==0x1b && i+1<n){uint8_t op=p[++i]; esc=true; if(op=='@') r.controls+="ESC @;"; else if((op=='a'||op=='E'||op=='t') && i+1<n){r.controls+="ESC "+String((char)op)+" "+String(p[++i])+";";} else r.controls+="ESC ?;"; continue;}
    if(c==0x1d && i+1<n){uint8_t op=p[++i];esc=true;if(op=='V' && i+1<n){r.controls+="GS V;";i++;}else if(op=='v' && i+3<n && p[i+1]=='0'){r.raster=true;r.controls+="RASTER IMAGE DATA;";break;}continue;}
    if(c=='\n'||c=='\r') {txt+='\n';continue;}
    if(c>=32) txt+=(char)c;
  }
  r.text=txt;
  if(r.raster){r.format="RASTER IMAGE DATA";return r;}
  if(txt.length() && validUtf8(txt)){r.decoded=true;r.format=esc?"ESC/POS UTF-8":"UTF-8 TEXT";}
  else if(esc) r.format="ESC/POS UNKNOWN TEXT";
  return r;
}
String PrintParser::hex(const uint8_t* p,size_t n,size_t offset){String s; char b[16];for(size_t i=0;i<n;i++){if(i%16==0){snprintf(b,sizeof b,"%04X: ",(unsigned)(i+offset));s+=b;}snprintf(b,sizeof b,"%02X ",p[i]);s+=b;if(i%16==15)s+='\n';}return s;}
