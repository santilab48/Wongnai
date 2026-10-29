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
