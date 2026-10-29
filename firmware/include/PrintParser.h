#pragma once
#include <Arduino.h>
struct ParsedPrint { String format="UNKNOWN FORMAT", text, controls; bool decoded=false, raster=false; };
class PrintParser { public: static ParsedPrint parse(const uint8_t* p,size_t n); static String hex(const uint8_t* p,size_t n,size_t offset=0); };
