# Test report

| Check | Result | Evidence |
|---|---|---|
| Python sender payload and chunked socket | PASS | `python -m unittest discover -s tools -v`: 5 tests |
| Firmware compile | BLOCKED | PlatformIO espressif32 platform download failed; no binary produced |
| Host stream assembler | PASS | `c++ -std=c++17 -Ifirmware/include firmware/test/host/stream_test.cpp -o /tmp/wongnai-stream-test && /tmp/wongnai-stream-test` |
| LCD/touch/Wi-Fi setup | BLOCKED | Exact board documentation and hardware absent |
| TCP capture on board | NOT TESTED | No hardware |
| Parser and queue on board | NOT TESTED | No hardware |
| Google Sheets/email | NOT TESTED | No configured deployment |
| 24-hour stress/POS integration | NOT TESTED | No hardware or authorized POS fixture |
