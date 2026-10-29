#pragma once
// UNVERIFIED FOR PCBFUN. Waveshare ESP32-S3-Touch-LCD-2.8 reference only.
// Never select this profile for PCBfun without comparing the actual schematic.
namespace WaveshareCandidate {
constexpr int lcdMosi=45,lcdSclk=40,lcdCs=42,lcdDc=41,lcdReset=39,backlight=5;
constexpr int touchSda=1,touchScl=3,touchInterrupt=4,touchReset=2;
constexpr unsigned char cst328Address=0x1a,cst3530Address=0x58;
}
