## 2024-05-18 - Avoid manual buffer loop for large stream reading in ESP32 HTTPClient
**Learning:** Manually allocating a buffer and looping `stream->readBytes` to download large binaries (like OTA firmware) via `HTTPClient` can cause memory fragmentation overhead, is blocking, and uses unnecessary CPU cycles copying the data in tight loops.
**Action:** Use `http.writeToStream(Stream*)` natively when saving HTTP streams directly to another Stream output (like SD Card `File`). It is highly optimized within the HTTPClient class.
## 2024-05-18 - Delete unused Arduino loopTask to save memory and CPU
**Learning:** In ESP32 Arduino sketches where all logic runs in custom FreeRTOS tasks and the main `loop()` function is empty, the default FreeRTOS `loopTask` created by the Arduino core continues to run unnecessarily, consuming ~8KB of heap stack memory and CPU cycles for context switching.
**Action:** Call `vTaskDelete(NULL);` at the end of the `setup()` function to destroy the `loopTask` when it is not needed.
