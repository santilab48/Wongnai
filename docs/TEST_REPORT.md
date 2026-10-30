# Test report

| Check | Result | Evidence |
|---|---|---|
| Python sender payload and chunked socket | PASS | `python -m unittest discover -s tools -v`: 5 tests |
| Firmware compile | PASS | `PLATFORMIO_CORE_DIR=/workspace/.platformio pio run -v` succeeded on 2026-10-29; compiler output: `/tmp/wongnai-pio-build.log`. Initial compile found three `min` type errors in Services.cpp, fixed before successful rebuild. Remaining warnings include `CORE_DEBUG_LEVEL` redefinition. |
| Firmware binary | PASS | `firmware/.pio/build/esp32s3/firmware.bin` generated after successful compile; SHA-256 `7dbcb1f739a8dec3f5b2756f9b3f851ec7123763ab1ae9c32fbb554bc73a8c79` |
| Installed dependencies | PASS | `pio pkg list`: espressif32 8.0.0, Arduino ESP32 4.30312.261024, ESP32-S3 libs 3.3.12, Xtensa/RISC-V toolchains 14.2.0+20260121, ArduinoJson 7.4.3. Full output: `/tmp/wongnai-pio-pkgs.log` |
| Host stream assembler | PASS | `c++ -std=c++17 -Ifirmware/include firmware/test/host/stream_test.cpp -o /tmp/wongnai-stream-test && /tmp/wongnai-stream-test` |
| Four simulated TCP clients | PASS | `python -m unittest discover -s tools -v` includes `test_concurrent_clients` against a host socket fixture; ESP32 listener remains untested. |
| 32 KiB boundary and binary preservation | NOT TESTED | Host tests cover 64 KiB assembler limit and 64 KiB binary socket transfer; exact 32 KiB firmware capture boundary has no dedicated test. |
| Stream timeout, queue restart and CRC, HTTPS acknowledgment | NOT TESTED | These firmware paths require a board or dedicated mocks; compile does not establish runtime behavior. |
| Event ID idempotency | NOT TESTED | Receiver lookup now searches only column A under a script lock; no Apps Script fixture or deployment test was run. |
| HTTPS email report | NOT TESTED | Firmware and Apps Script report request/ack code reviewed; no configured gateway, token, recipient, or deployment test. |
| Capture during cloud failures | NOT TESTED | Capture and cloud tasks are separate in firmware; board-level responsiveness under failures has not been measured. |
| LCDWIKI candidate and diagnostic compile | PASS | Separate `pio run -e lcdwiki_es3c28p_candidate` and `pio run -e lcdwiki_es3c28p_diagnostic` succeeded; logs `/tmp/wongnai-lcdwiki-candidate.log` and `/tmp/wongnai-lcdwiki-diagnostic.log`. |
| LCDWIKI LCD/touch hardware validation | NOT TESTED | Physical ES3C28P revision has not been verified; no flashing or hardware test performed. |
| TCP capture on board | NOT TESTED | No hardware; Python socket tests do not validate ESP32 listener |
| Parser and queue on board | NOT TESTED | No hardware |
| Google Sheets/email | NOT TESTED | No configured deployment or provisioned CA/token |
| 24-hour stress/POS integration | NOT TESTED | No hardware or authorized POS fixture |

## Initial protocol capture interface (2026-10-29)

| Check | Result | Evidence |
|---|---|---|
| LCDWIKI Live Capture build | PASS | `pio run -e lcdwiki_es3c28p_candidate`; `/tmp/wongnai-live-normal.log` |
| LCDWIKI diagnostic build | PASS | `pio run -e lcdwiki_es3c28p_diagnostic`; `/tmp/wongnai-live-diagnostic.log` |
| Host UI ordering, touch selection, HEX/TEXT, binary buffer | PASS | `c++ -std=c++17 -Ifirmware/test/host/stubs -Ifirmware/include firmware/src/UiModel.cpp firmware/test/host/ui_test.cpp -o /tmp/wongnai-ui-test && /tmp/wongnai-ui-test` |
| Real TCP events on physical LCD; touch and Wi-Fi provisioning | NOT TESTED | No verified PCBfun board available; LCDWIKI remains a candidate profile. |
| TCP latency during UI refresh and cloud failure | NOT TESTED | The loop polls capture before UI; LittleFS reads and LCD drawing can still pause polling. Hardware timing evidence is required. |
| Raw data after cloud acknowledgment | NOT TESTED | Current queue removes records on acknowledgment, so inspection is available only while the record remains pending. |

## Persistent archive split (2026-10-29)

| Check | Result | Evidence |
|---|---|---|
| Normal LCDWIKI candidate compile | PASS | `pio run -e lcdwiki_es3c28p_candidate`; `/tmp/wongnai-archive-normal.log` |
| LCDWIKI diagnostic compile | PASS | `pio run -e lcdwiki_es3c28p_diagnostic`; `/tmp/wongnai-archive-diagnostic.log` |
| Archive record CRC, 32 KiB boundary, binary bytes, capacity guard | PASS | `c++ -std=c++17 -Ifirmware/include firmware/test/host/archive_format_test.cpp -o /tmp/wongnai-archive-test && /tmp/wongnai-archive-test` |
| History navigation and raw HEX/TEXT UI | PASS | `c++ -std=c++17 -Ifirmware/test/host/stubs -Ifirmware/include firmware/src/UiModel.cpp firmware/test/host/ui_test.cpp -o /tmp/wongnai-ui-test && /tmp/wongnai-ui-test` |
| Host archive file reopen, binary integrity, CRC corruption | PASS | `c++ -std=c++17 -Ifirmware/include firmware/test/host/archive_persistence_test.cpp -o /tmp/wongnai-archive-persistence-test && /tmp/wongnai-archive-persistence-test`; uses the firmware archive format on a host file. |
| LittleFS restart and power-loss recovery | NOT TESTED | Recovery and legacy queue migration compile, but no LittleFS host fixture or physical board test was run. |
| Cloud acknowledgment preserves archive | NOT TESTED | Code removes only the `.pending` marker after validated acknowledgment; no live cloud or filesystem fault test was run. |
| Full storage on board and TCP latency during Flash scans | NOT TESTED | Capacity guard has a host test; no board timing or Flash saturation test was run. |

## LCDWIKI ES3C28P N16R8 hardware configuration (2026-10-30)

| Check | Result | Evidence |
|---|---|---|
| Manufacturer hardware identification and GPIO mapping | PASS | [LCDWIKI ES3C28P product page](https://www.lcdwiki.com/2.8inch_ESP32-S3_Display) specifies N16R8, ILI9341V, FT6336G and the matching LCD/touch GPIO assignments. |
| PlatformIO board selection and framework compatibility | PASS | PlatformIO `espressif32` 8.0.0 board `4d_systems_esp32s3_gen4_r8n16.json` defines 16 MB flash, `qio_opi`, `BOARD_HAS_PSRAM`, and variant `esp32_s3r8n16`; installed Arduino ESP32 3.3.12 contains the variant and compiled both environments. It is selected for matching N16R8 silicon/memory only; the display wiring remains LCDWIKI-specific. |
| Partitions fit 16 MB flash | PASS | `partitions.csv`: last partition ends at `0xcb0000 + 0x350000 = 0x1000000` (16,777,216 bytes). |
| Candidate build for Wi-Fi/Live TCP Capture testing | PASS | Rebuilt with `pio run -e lcdwiki_es3c28p_candidate` on the configured N16R8 target; normal UI/application environment, build only. |
| Diagnostic build | PASS | `pio run -e lcdwiki_es3c28p_diagnostic` on the configured N16R8 target. Boot diagnostic reads chip model/revision/cores, detected flash size/mode/bus frequency, PSRAM initialization and total size, and free heap via Arduino ESP32 runtime APIs. |
| Hardware identification and flash | PASS (reported hardware test) | ESP32-S3 detected; 16 MB Flash reported. |
| LCD diagnostic screen | PASS (reported hardware test) | LCD diagnostic screen displayed successfully on the physical LCDWIKI unit. |
| FT6336G touch detection | PASS (reported hardware test) | FT6336G detected by the diagnostic firmware. |
| Touch coordinate reporting | PASS (reported hardware test) | Serial Monitor received touch coordinates `(105,142)` and `(183,298)`. |
| Touch orientation/calibration | NOT FULLY VERIFIED | Coordinates were received, but screen orientation and touch-to-screen coordinate alignment have not been fully validated. |
| PSRAM runtime initialization and capacity | NOT FULLY VERIFIED | The hardware report does not yet establish successful runtime initialization or verify the full 8 MB capacity. |
| Wi-Fi and Live TCP Capture on hardware | PENDING | Normal firmware has been built for the next test. No upload or deployment was performed in this step. |
| Runtime PSRAM interface mode | LIMITATION | The framework configures OPI via `qio_opi`; runtime diagnostics confirm initialization and detected capacity, but do not independently identify the physical PSRAM bus mode. |
| Touch I2C address | LIMITATION | Profile address `0x38` is not specified on the cited LCDWIKI product page; confirm using the physical board/bus scan if touch detection fails. |
| Compiler warnings | PASS WITH WARNINGS | Builds report the existing `CORE_DEBUG_LEVEL` redefinition and deprecated `NetworkServer::available()` warnings; no warning remains from the new runtime diagnostic formatting. |

## LCD flicker and text overlap remediation

| Check | Result | Evidence |
|---|---|---|
| Normal LCDWIKI candidate build | PASS | `pio run -e lcdwiki_es3c28p_candidate`; completed successfully after the UI rendering changes. |
| LCDWIKI diagnostic build | PASS | `pio run -e lcdwiki_es3c28p_diagnostic`; completed successfully after the UI rendering changes. |
| Dirty-region redraw | IMPLEMENTED | `UiModel` now caches displayed values, clears only changed text rectangles, and clears the full screen only on a page transition. Text wrapping is disabled and strings are clipped to their regions. |
| UI drawing ownership and synchronization | IMPLEMENTED | `DisplayManager::tick()` performs LCD drawing; touch events and model access are serialized by a mutex. Storage reads happen outside the UI lock. |
| Refresh cadence | IMPLEMENTED | Normal UI rendering is capped at 5 Hz; storage-backed event refresh remains at 0.5 Hz. No deliberate delay was added to the TCP loop. |
| Event ordering and layout | IMPLEMENTED | Events are sorted newest-first by timestamp (ID descending for timestamp ties). Raw-data rows use compact spacing and remain above the footer. |
| Focused host UI tests | NOT RUN | Tests cover unchanged-frame redraw suppression, clear-before-redraw ordering, newest-first records, navigation, and raw HEX rendering. The current Windows environment has no `cl`, `clang++`, or `g++` host compiler available. |
| LCD flicker/overlap on hardware | PENDING | Firmware builds only; visual revalidation on the ESP32-S3 board is still required. No upload, erase, or deployment was performed. |

## TCP capture and archive visibility investigation (2026-10-30)

| Check | Result | Evidence |
|---|---|---|
| TCP disconnect finalization path | CODE VERIFIED | A non-empty stream is finalized as complete when the client disconnects with no buffered bytes. A 5-second idle timeout finalizes it as incomplete. Added a host case for `TEST ORDER 003` (14 bytes) followed by a clean disconnect. |
| Live Capture archive source | BUG FIXED | Live Capture previously read `uploadQueue(4)`, so acknowledgment removed its queue marker and hid the event although the archive `.rec` remained. Live and History now both read `history(4)`; pending count remains separately sourced from the upload queue. |
| Capture/archive/UI diagnostics | IMPLEMENTED | Logs cover listener status, first poll execution, client accept, received chunks, finalization, archive save/error, archive mount/history query failures, and UI archive state changes. Startup reports storage and listener initialization failures. |
| Focused host disconnect test | NOT RUN | Added to `firmware/test/host/stream_test.cpp`; this Windows environment has no native C++ compiler available. |
| Normal LCDWIKI candidate build | PASS | `pio run -e lcdwiki_es3c28p_candidate`; succeeded after TCP/archive changes. |
| LCDWIKI diagnostic build | PASS | `pio run -e lcdwiki_es3c28p_diagnostic`; succeeded after TCP/archive changes. |
| Reported physical TCP test (192.168.1.102:9100) | UNRESOLVED | Observed `[UI] records=0` proves the UI refresh and archive query path executed and returned no visible records. Existing firmware logs did not distinguish listener startup, polling, receive, finalization, or write failures; runtime success/failure of those stages cannot be inferred from that UI line. |
| UI zero-record log cadence | FIXED | The prior log ran on every archive refresh (every 2 seconds). It now logs only when visible count, page, or newest record ID changes. |
| Listener status and poll diagnostics | IMPLEMENTED | `begin()` now checks the framework server listening status rather than logging success unconditionally; the loop emits `[TCP] poll active` once when `tick()` first executes. ESP32 Arduino `NetworkServer::accept()` is used instead of deprecated `available()`. |
| Normal LCDWIKI candidate build | PASS | `pio run -e lcdwiki_es3c28p_candidate`; runtime-diagnostic changes compiled successfully. |
| Host disconnect test | NOT RUN | `firmware/test/host/stream_test.cpp` includes the 14-byte payload followed by disconnect; native host compiler unavailable in the current Windows environment. |
| Hardware retest | PENDING | No upload, Flash erase, or deployment was performed. |

## LittleFS archive directory diagnostics (2026-10-30)

| Check | Result | Evidence |
|---|---|---|
| LittleFS mount configuration | FIXED | Firmware now explicitly mounts partition label `spiffs` at `/littlefs` with `formatOnFail=false`; it validates the matching data/SPIFFS partition first and logs its offset and size. |
| Partition label and geometry | PASS | `partitions.csv` defines `spiffs,data,spiffs,0xcb0000,0x350000`; it ends at `0x1000000` (16 MiB), matching the 16 MB LCDWIKI target. |
| LittleFS directory API support | PASS | Installed Arduino-ESP32 3.3.12 LittleFS uses `FS::mkdir`, VFS-backed `File::isDirectory`, and `openNextFile`; the API supports the `/a` and `/q` create/open/iterate flow. |
| First-boot directory handling | IMPLEMENTED | Missing `/a` and `/q` are created only after a successful mount, and each directory is reopened and checked before archive recovery begins. Existing paths that are files and failed create/open attempts produce distinct one-shot diagnostics. |
| Host lifecycle fixture | ADDED; NOT RUN | `firmware/test/host/archive_filesystem_test.cpp` covers the shared mount/directory helper, failed mount without formatting, empty first boot, directory creation/open validation, and binary archive write/reopen using a `std::filesystem` fixture. It does not execute the full `CaptureStorage` recovery path or real LittleFS. The local Zig C++ driver could not resolve the test source (`FileNotFound`), so these host assertions were not executed. |
| LCD state during TCP receive | IMPLEMENTED | Empty history now displays `Receiving TCP data` while a client is active; a zero-capacity archive displays `Flash unavailable` rather than a misleading `0/0K FULL`. |
| Storage failure while TCP is available | CODE VERIFIED | TCP setup remains independent of storage mount success. Append rejects an unavailable archive with a reason, and the server logs that the payload was received but not persisted. No board-level TCP receive test was performed. |
| Normal LCDWIKI candidate build | PASS | `pio run -e lcdwiki_es3c28p_candidate`; final code built successfully on 2026-10-30. |
| LCDWIKI diagnostic build | PASS | `pio run -e lcdwiki_es3c28p_diagnostic`; final code built successfully on 2026-10-30. |
| Actual board mount status and archive directory contents | NOT VERIFIED | The supplied `[ARCHIVE] ERROR directory open failed path=/a` does not prove a successful mount: the prior code ignored the `begin()` result in later list calls. The complete boot log or a safe read-only board retest is required to establish actual runtime state. No flash, format, or upload was performed. |
