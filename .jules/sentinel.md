## 2025-03-01 - Insecure HTTP for Firmware OTA Updates
**Vulnerability:** The ESP32 device downloaded its firmware updates over an insecure HTTP connection using `WiFiClient`, which is vulnerable to Man-In-The-Middle (MITM) attacks. An attacker could intercept the request and inject malicious firmware.
**Learning:** For remote updates, always establish a secure TLS connection. Using `WiFiClientSecure` without `setInsecure()` and properly validating the remote server's certificate is required to verify the server's identity and ensure the downloaded payload is trustworthy.
**Prevention:** Always use `WiFiClientSecure` and `client.setCACert(rootCACertificate)` with the appropriate root CA for remote server requests. Avoid downloading sensitive or executable data over plain HTTP.
