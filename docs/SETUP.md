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

The [LCDWIKI 2.8inch ESP32-S3 Display](https://www.lcdwiki.com/2.8inch_ESP32-S3_Display) identifies the ES3C28P as an ESP32-S3 N16R8: 16 MB external SPI flash and 8 MB internal OPI PSRAM, with an ILI9341V 240 × 320 SPI display and FT6336G I2C touch controller. Its documented GPIO mapping matches `firmware/include/board_profiles/lcdwiki_es3c28p_candidate.h`: LCD CS 10, DC 46, SCLK 12, MOSI 11, MISO 13, backlight 45 (active high), and LCD reset shared with the ESP32-S3 module reset; touch SDA 16, SCL 15, reset 18 (active low), and interrupt 17.

The candidate and diagnostic PlatformIO environments use the installed `4d_systems_esp32s3_gen4_r8n16` N16R8 board definition as the matching silicon/memory target (not as a claim that the LCDWIKI PCB is a 4D Systems board). With installed PlatformIO `espressif32` 8.0.0 and Arduino ESP32 3.3.12, its `esp32_s3r8n16` variant is available and the board definition selects QIO flash, `qio_opi` memory, and `BOARD_HAS_PSRAM`; candidate overrides retain the project's 16 MB partition table. The LCD/touch adapters remain the board-specific ES3C28P wiring. The diagnostic environment prints the detected chip model/revision/core count, flash size/mode/bus frequency, PSRAM initialization result and total, and free heap at boot. Detected runtime values are not substituted with nominal board specifications.

Build both environments from `firmware/` with `pio run -e lcdwiki_es3c28p_candidate` and `pio run -e lcdwiki_es3c28p_diagnostic`. The custom partition table occupies exactly 16 MiB and ends at offset `0x1000000`. Compile results do not prove physical flash/PSRAM operation: connect an actual ES3C28P and inspect the diagnostic serial output to confirm 16 MiB flash, QIO operation, successful PSRAM initialization with 8 MiB available, LCD output/backlight/reset, and touch response/orientation. The FT6336G `0x38` address is used by the profile but is not stated in the cited LCDWIKI page. Do not upload until the physical board and runtime report have been checked.

## Initial capture interface

After Wi-Fi connects, the LCDWIKI candidate build opens Live Capture. It shows the latest pending LittleFS capture records (newest first), UTC timestamp when available, IP, byte count, parser format, and saved/incomplete status. Tap a row for the first 96 bytes in HEX or printable ASCII TEXT; these views never rewrite the original binary record. All entries sourced from the real Flash queue are labeled RECEIVED; simulated host fixtures are never injected into the device UI. The WiFi button opens an on-screen uppercase letter/digit keyboard for SSID and password, then Connect stores credentials in NVS. The status line shows the TCP listener and IP address. The UI is a protocol inspection interface, not a parsed order dashboard. Records are removed after cloud acknowledgment, and physical timing, touch calibration, and Wi-Fi provisioning remain untested.

## Capture archive and upload queue

Original captures are stored as CRC-checked binary `.rec` files under LittleFS `/a`. The separate `/q` directory holds `.pending` markers for cloud delivery. A successful authenticated cloud acknowledgment removes only its marker; History can still inspect the archived payload. Startup scans the archive as its index, validates CRC before exposing a record, recovers staged records with pending markers, and migrates legacy queue records. Duplicate IDs are rejected. No automatic archive eviction occurs: once free Flash falls below the reserved margin, new captures fail and the UI shows a storage-full warning. The display shows used/total internal Flash and History includes uploaded entries. No SD card is used. Physical power-loss and full-Flash behavior remain unverified.
