# Test report

| Check | Result | Evidence |
|---|---|---|
| Python sender payload and chunked socket | PASS | `python -m unittest discover -s tools -v`: 5 tests |
| Firmware compile | BLOCKED | PlatformIO 6.2.0 and espressif32 platform 8.0.0 installed; toolchain download from api.registry.platformio.org fails with HTTPClientError; no binary produced |
| Host stream assembler | PASS | `c++ -std=c++17 -Ifirmware/include firmware/test/host/stream_test.cpp -o /tmp/wongnai-stream-test && /tmp/wongnai-stream-test` |
| LCD/touch/Wi-Fi setup | BLOCKED | Exact board documentation and hardware absent |
| TCP capture on board | NOT TESTED | No hardware; Python socket tests do not validate ESP32 listener |
| Parser and queue on board | NOT TESTED | No hardware |
| Google Sheets/email | NOT TESTED | No configured deployment or provisioned CA/token |
| 24-hour stress/POS integration | NOT TESTED | No hardware or authorized POS fixture |
