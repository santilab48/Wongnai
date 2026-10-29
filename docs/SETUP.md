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
