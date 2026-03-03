/**
    * ESP32
    * CS        -> 5
    * MOSI    -> 23
    * SCK     -> 18
    * MISO    -> 19
    */

// Include necessary libraries
#include <WiFi.h>           // WiFi library for connecting to the internet
#include <WiFiClientSecure.h> // HTTPS client library
#include <HTTPClient.h>     // HTTP client library for making HTTP requests
#include <SPI.h>            // SPI library for communication with peripherals like SD cards
#include <SD.h>             // SD library for handling file operations on SD cards
#include <Update.h>         // Library for handling firmware updates over-the-air (OTA)

// WiFi network credentials
const char *ssid = "WiFi_NAME";
const char *password = "PASSWORD";

// Server information for checking and downloading firmware updates
const char *updateURL = "https://ota-demo-74718.web.app/firmware.bin";
// example: http://example.com/firmware-update?uuid=uuid&version=0.0.1

// 🛡️ Sentinel: Google Trust Services Root R1 for secure OTA downloads (prevents MITM attacks)
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n" \
"MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n" \
"A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n" \
"27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n" \
"Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n" \
"TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n" \
"qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n" \
"szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n" \
"Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n" \
"MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n" \
"wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n" \
"aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n" \
"VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n" \
"AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n" \
"FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n" \
"C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n" \
"QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n" \
"h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n" \
"7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n" \
"ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n" \
"MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n" \
"Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n" \
"6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n" \
"0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n" \
"2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n" \
"bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n" \
"-----END CERTIFICATE-----\n";

// Flags to control download and update status
bool is_downloading = false;
bool is_updating = false;

// Pin configuration for the SD card reader
#define SD_CS_PIN 5

// Define timeouts for SD and WiFi operations
#define SD_Timeout 60000            // 60 seconds for SD operations
#define WiFi_Timeout 3600000        // 3600 seconds (1 hour) for WiFi operations

// Function to perform firmware update from a binary file
void performUpdate(Stream &updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {    // Start the update process
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize) {
            Serial.println("Written : " + String(written) + " successfully");
        } else {
            Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end()) {    // Finalize the update process
            Serial.println("OTA done!");
            if (Update.isFinished()) {
                Serial.println("Update successfully completed. Rebooting.");
                ESP.restart();    // Restart the ESP to load the new firmware
            } else {
                Serial.println("Update not finished? Something went wrong!");
            }
        } else {
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }
    } else {
        Serial.println("Not enough space to begin OTA");
    }
}

// Function to update firmware from the SD card
void UpdateFirmwareFromSD() {
    if (is_downloading) {
        Serial.println("Still downloading new firmware.");
        return;
    }

    if (!SD.begin(SD_CS_PIN)) {    // Initialize the SD card
        Serial.println("Failed to initialize SD card.");
        return;
    }

    File binFile = SD.open("/update.bin", FILE_READ);    // Open the binary file for reading
    if (!binFile) {
        Serial.println("No update found.");
        return;
    }

    is_updating = true;
    Serial.println("SD card initialized successfully.");

    size_t updateSize = binFile.size();
    if (updateSize > 0) {
        Serial.println("Try to start update");
        performUpdate(binFile, updateSize);    // Perform the update process
    } else {
        Serial.println("Error, file is empty");
    }

    binFile.close();    // Close the binary file
    SD.remove("/update.bin");   // Delete the binary file after update

    is_updating = false;
}

// Function to connect to WiFi
void WiFiConnection() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {    // Wait for WiFi connection
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected successfully!");
}

// Function to check for new firmware updates via HTTP
void CheckAPIForUpdates() {
    if (is_updating) {
        Serial.println("Still updating.");
        return;
    }

    Serial.println("Checking for updates...");

    WiFiClientSecure client;
    client.setCACert(rootCACertificate); // 🛡️ Sentinel: Enforce strict TLS validation to prevent MITM
    HTTPClient http;

    if (http.begin(client, updateURL)) {    // Start HTTP connection
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_ALREADY_REPORTED) {
            Serial.println("Firmware already up to date.");

        } else if (httpCode == HTTP_CODE_OK) { // 🛡️ Sentinel: Reject 301 redirects to prevent writing HTML payloads to firmware binary
            is_downloading = true;

            Serial.println("Update found!");
            String fileName = "/update.bin";    // Define file name for new firmware

            if (!SD.begin(SD_CS_PIN)) {    // Re-initialize SD card
                Serial.println("Failed to initialize SD card.");
                is_downloading = false;
                return;
            }

            File file = SD.open(fileName, FILE_WRITE);    // Open file for writing
            if (file) {
                // 🛡️ Sentinel: Fetch expected file size to validate download integrity
                int expectedSize = http.getSize();

                // ⚡ Bolt: Replaced manual readBytes loop with http.writeToStream
                // 💡 What: Used native HTTPClient writeToStream method
                // 🎯 Why: Avoids blocking manual buffer allocation, CPU loops, and reduces fragmentation overhead
                // 📊 Impact: Significantly faster download speeds and lower peak memory usage for large OTA binaries
                int written = http.writeToStream(&file);
                file.close();    // Close the file

                // 🛡️ Sentinel: Validate that the complete file was downloaded to prevent flashing corrupted/truncated firmware (DoS risk)
                // expectedSize will be -1 if the server uses chunked transfer encoding
                if (written > 0 && (expectedSize == -1 || written == expectedSize)) {
                    Serial.println("File saved successfully.");
                } else {
                    Serial.println("Error saving file or incomplete download. Deleting invalid payload.");
                    SD.remove(fileName); // Remove incomplete firmware
                }
            } else {
                Serial.print("Error creating file: ");
                Serial.println(fileName);
            }

            http.end();    // Close HTTP connection
        } else {
            Serial.print("Request canceled. HTTP Code : ");
            Serial.println(httpCode);
        }

        is_downloading = false;
    } else {
        Serial.println("HTTP connection error!");
    }
}

void setup() {
    Serial.begin(115200);   // Start the Serial communication
    WiFiConnection();   // Establish WiFi connection

    // Task creation and configuration for periodic update checks
    static TaskHandle_t WiFiUpdaterTask;
    static StackType_t WiFiUpdaterTaskStack[10240];
    static StaticTask_t WiFiUpdaterTaskBuffer;

    xTaskCreateStaticPinnedToCore(
        WiFiUpdaterLoop,    // Function to check updates via WiFi
        "WiFiUpdaterLoop",
        sizeof(WiFiUpdaterTaskStack) / sizeof(WiFiUpdaterTaskStack[0]),
        NULL,
        1,
        WiFiUpdaterTaskStack,
        &WiFiUpdaterTaskBuffer,
        1    // Core number where the task should run
    );

    static TaskHandle_t SDUpdaterTask;
    static StackType_t SDUpdaterTaskStack[10240];
    static StaticTask_t SDUpdaterTaskBuffer;

    xTaskCreateStaticPinnedToCore(
        SDUpdaterLoop,    // Function to check and perform updates from SD card
        "SDUpdaterLoop",
        sizeof(SDUpdaterTaskStack) / sizeof(SDUpdaterTaskStack[0]),
        NULL,
        1,
        SDUpdaterTaskStack,
        &SDUpdaterTaskBuffer,
        1    // Core number where the task should run
    );
}

void loop(void) {
    // Empty loop - all operations are handled by tasks
}

void WiFiUpdaterLoop(void *pcParameters) {
    for (;;) {
        CheckAPIForUpdates();
        delay(WiFi_Timeout);
    }
}

void SDUpdaterLoop(void *pcParameters) {
    for (;;) {
        delay(SD_Timeout);
        UpdateFirmwareFromSD();
    }
}
