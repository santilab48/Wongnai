#pragma once
// LCDWIKI ES3C28P family candidate. Confirm the physical board revision before flashing.
namespace LcdwikiEs3c28p {
constexpr int width=240, height=320;
constexpr int lcdCs=10, lcdDc=46, lcdSclk=12, lcdMosi=11, lcdMiso=13, backlight=45;
// LCD reset is wired to CHIP_PU / EN; it is not software-controlled.
constexpr int touchSda=16, touchScl=15, touchReset=18, touchInterrupt=17;
constexpr unsigned char touchAddress=0x38;
}
