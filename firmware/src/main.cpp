#include <Arduino.h>
#include <esp_task_wdt.h>
#include "CaptureServer.h"
#include "Services.h"
static CaptureStorage storage;static CaptureServer capture;static WiFiManager wifi;static GoogleSheetsUploader uploader;static EmailReporter email;static DisplayManager display;static TouchInput touch;static TimeManager clockService;static Diagnostics diagnostics;
static void cloudTask(void*){for(;;){uploader.tick();email.tick(diagnostics);vTaskDelay(pdMS_TO_TICKS(1000));}}
void setup(){Serial.begin(115200);storage.begin();wifi.begin();clockService.begin();display.begin();touch.attach(&display.model());touch.begin();email.begin();uploader.begin(&storage);capture.begin(9100,&storage);xTaskCreatePinnedToCore(cloudTask,"cloud",12288,nullptr,1,nullptr,0);esp_task_wdt_config_t cfg={.timeout_ms=10000,.idle_core_mask=0,.trigger_panic=true};esp_task_wdt_init(&cfg);esp_task_wdt_add(NULL);}
void loop(){capture.tick();wifi.tick();touch.tick();display.tick(diagnostics,storage);esp_task_wdt_reset();delay(1);}
