# Wongnai Order Capture V1 development status

This is an experimental, direct-addressed TCP receiver. It receives only traffic sent to its own IP:9100 or explicitly forwarded by an authorized sender. It does not see unrelated printer unicast traffic.

## Build and flash

Install PlatformIO, then run `python -m platformio run -d firmware`. The selected `esp32-s3-devkitc-1` target is only a generic ESP32-S3 compilation target, **not** a verified PCBfun board definition. Do not flash until the exact PCBfun board revision, LCD controller, touch controller, GPIOs, flash mode, PSRAM mode, bootloader and partition layout have been checked against vendor documentation. After verification, flash with `python -m platformio run -d firmware -t upload` and monitor serial output with `python -m platformio device monitor -d firmware`. Flashing has not been performed.

## Wi-Fi and Google

Wi-Fi settings are stored in NVS namespace `wifi`, keys `ssid` and `password`. Without credentials, the firmware starts an AP named `Wongnai-Setup`, but AP configuration and on-screen keyboard are not implemented. NVS namespace `upload` expects keys `url`, `token`, and `ca` (PEM root CA certificate); an authenticated device provisioning UI is still required. The uploader refuses upload without a configured CA certificate; no secure provisioning UI exists yet. Never place credentials in source control.

Create a spreadsheet with an `Events` sheet and a Drive folder for raw bytes. Paste `apps-script/Code.gs` into Apps Script and set script properties `CAPTURE_TOKEN`, `SHEET_ID`, `RAW_FOLDER_ID`, and `REPORT_EMAIL`. Deploy as a Web App with access limited to authorized users. Configure a daily trigger for `sendDailyReport`. A server ACK contains `ok` and the matching `eventId`; only then does the firmware delete the local queue record.

## Test sender

`python tools/tcp_sender.py DEVICE_IP --kind escpos --chunk 2` sends a split ESC/POS fixture. Other kinds: `english`, `thai`, `binary`, `invalid`. Only run against a device and network you are authorized to test.

## Known limitations

- LCD/touch controller and pinout unverified; display and touch adapters are placeholders. Dashboard, history and raw views are not operational.
- Four active TCP clients are supported with 32 KiB per job; further clients and oversized jobs are rejected with an error counter. Chunk boundaries are preserved within a connection, but idle timeout splits a job.
- Wi-Fi AP cannot yet accept configuration; static IP and connectivity probe are absent.
- Queue has CRC and temporary-file commit but no dedicated queue manifest, durability audit or wear benchmark. Flash partition size is provisional.
- ESC/POS parser handles selected commands and UTF-8; Thai legacy code pages and full raster length parsing are absent. It does not infer order fields.
- Apps Script stores raw data as a Drive file, but upload size/latency limits and Drive orphan cleanup need testing. The token is sent in JSON body, so TLS certificate verification is mandatory before use.
- Firmware report POST runs in a separate FreeRTOS cloud task and requires NVS `email` keys `url`, `token`, `ca`, `recipient`; Apps Script only sends to the configured `REPORT_EMAIL`. Provisioning UI and real endpoint validation remain outstanding.
- No hardware tests, live Google upload, 24-hour soak, POS integration or load test has run.
- A future authorized print gateway could forward a copy to this device without changing printer traffic; it is not implemented.

## Candidate hardware reference

The attached hardware reference identifies the **Waveshare** ESP32-S3-Touch-LCD-2.8 as a candidate only. Its ST7789 and SPI pins are recorded in `firmware/include/board_profiles/waveshare_esp32s3_touch_lcd_28_candidate.h`, which is not enabled. Waveshare V1 uses CST328 touch, while V2 uses CST3530. Neither controller nor any pin mapping is verified for PCBfun. A vendor schematic or actual board inspection is required before enabling a display/touch driver.

## LCDWIKI ES3C28P candidate

The [LCDWIKI 2.8inch ESP32-S3 Display](https://www.lcdwiki.com/2.8inch_ESP32-S3_Display) is the source for the ES3C28P family identification. The candidate profile uses the supplied ILI9341V SPI map (CS 10, DC 46, SCLK 12, MOSI 11, MISO 13, backlight 45; reset on CHIP_PU/EN) and FT6336G I2C map (SDA 16, SCL 15, RST 18, INT 17). The display is 240 × 320. Select `lcdwiki_es3c28p_candidate` to compile the UI adapter, or `lcdwiki_es3c28p_diagnostic` for a blue LCD test screen and touch coordinates on Serial. The latter is intended for later bench testing only. The physical board revision, flash geometry, backlight polarity, touch orientation, and display behavior remain unverified. The Waveshare profile remains present and disabled.

Build with `PLATFORMIO_CORE_DIR=/workspace/.platformio pio run -e lcdwiki_es3c28p_candidate` or `pio run -e lcdwiki_es3c28p_diagnostic` from `firmware/`. Do not flash until the physical board is verified.

## Initial capture interface

After Wi-Fi connects, the LCDWIKI candidate build opens Live Capture. It shows the latest pending LittleFS capture records (newest first), UTC timestamp when available, IP, byte count, parser format, and saved/incomplete status. Tap a row for the first 96 bytes in HEX or printable ASCII TEXT; these views never rewrite the original binary record. All entries sourced from the real Flash queue are labeled RECEIVED; simulated host fixtures are never injected into the device UI. The WiFi button opens an on-screen uppercase letter/digit keyboard for SSID and password, then Connect stores credentials in NVS. The status line shows the TCP listener and IP address. The UI is a protocol inspection interface, not a parsed order dashboard. Records are removed after cloud acknowledgment, and physical timing, touch calibration, and Wi-Fi provisioning remain untested.

## Capture archive and upload queue

Original captures are stored as CRC-checked binary `.rec` files under LittleFS `/a`. The separate `/q` directory holds `.pending` markers for cloud delivery. A successful authenticated cloud acknowledgment removes only its marker; History can still inspect the archived payload. Startup scans the archive as its index, validates CRC before exposing a record, recovers staged records with pending markers, and migrates legacy queue records. Duplicate IDs are rejected. No automatic archive eviction occurs: once free Flash falls below the reserved margin, new captures fail and the UI shows a storage-full warning. The display shows used/total internal Flash and History includes uploaded entries. No SD card is used. Physical power-loss and full-Flash behavior remain unverified.
