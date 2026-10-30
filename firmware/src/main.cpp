#include <Arduino.h>
#include <esp_task_wdt.h>
#include "CaptureServer.h"
#include "Services.h"
static CaptureStorage storage;static CaptureServer capture;static WiFiManager wifi;static GoogleSheetsUploader uploader;static EmailReporter email;static DisplayManager display;static TouchInput touch;static TimeManager clockService;static Diagnostics diagnostics;
static void cloudTask(void*){for(;;){uploader.tick();email.tick(diagnostics);vTaskDelay(pdMS_TO_TICKS(1000));}}
static void configureTaskWatchdog(){
 esp_task_wdt_config_t cfg={.timeout_ms=10000,.idle_core_mask=0,.trigger_panic=true};
 esp_err_t err=esp_task_wdt_init(&cfg);
 if(err==ESP_ERR_INVALID_STATE)Serial.println("Task watchdog already initialized; preserving existing configuration");
 else if(err!=ESP_OK){Serial.printf("Task watchdog initialization failed: %s (%d)\n",esp_err_to_name(err),err);return;}
 err=esp_task_wdt_status(NULL);
 if(err==ESP_OK){Serial.println("Current task already subscribed to task watchdog");return;}
 if(err!=ESP_ERR_NOT_FOUND){Serial.printf("Task watchdog status check failed: %s (%d)\n",esp_err_to_name(err),err);return;}
 err=esp_task_wdt_add(NULL);
 if(err!=ESP_OK){Serial.printf("Task watchdog subscription failed: %s (%d)\n",esp_err_to_name(err),err);return;}
 Serial.println("Current task subscribed to task watchdog");
}
void setup(){Serial.begin(115200);
#ifdef LCDWIKI_DIAGNOSTIC
 diagnostics.printHardwareReport();
#endif
 if(!storage.begin())Serial.println("[BOOT] capture archive unavailable; incoming captures cannot be saved");
 wifi.begin();clockService.begin();display.begin();touch.attach(&display);touch.begin();email.begin();uploader.begin(&storage);if(!capture.begin(9100,&storage))Serial.println("[BOOT] TCP capture server failed to start");xTaskCreatePinnedToCore(cloudTask,"cloud",12288,nullptr,1,nullptr,0);configureTaskWatchdog();}
void loop(){capture.tick();wifi.tick();touch.tick();String ssid,password;if(display.takeConnectRequest(ssid,password))wifi.configure(ssid,password);display.tick(diagnostics,storage,diagnostics.state(true,capture.receiving(),storage.freeBytes(),storage.available()));esp_task_wdt_reset();delay(1);}
