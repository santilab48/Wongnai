#include <Arduino.h>
#include <esp_task_wdt.h>
#include "CaptureServer.h"
#include "Services.h"
static CaptureStorage storage;static CaptureServer capture;static WiFiManager wifi;static GoogleSheetsUploader uploader;static EmailReporter email;static DisplayManager display;static TouchInput touch;static TimeManager clockService;static Diagnostics diagnostics;
void setup(){Serial.begin(115200);storage.begin();wifi.begin();clockService.begin();display.begin();touch.begin();email.begin();uploader.begin(&storage);capture.begin(9100,&storage);esp_task_wdt_config_t cfg={.timeout_ms=10000,.idle_core_mask=0,.trigger_panic=true};esp_task_wdt_init(&cfg);esp_task_wdt_add(NULL);}
void loop(){capture.tick();wifi.tick();touch.tick();display.tick(diagnostics,storage);static uint32_t last=0;if(millis()-last>1000){last=millis();uploader.tick();email.tick(diagnostics);}esp_task_wdt_reset();delay(1);}
