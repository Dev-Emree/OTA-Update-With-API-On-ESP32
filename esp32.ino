/**
    * ESP32
    * CS        -> 5
    * MOSI    -> 23
    * SCK     -> 18
    * MISO    -> 19
    */

// Include necessary libraries
#include <WiFi.h>           // WiFi library for connecting to the internet
#include <HTTPClient.h>     // HTTP client library for making HTTP requests
#include <SPI.h>            // SPI library for communication with peripherals like SD cards
#include <SD.h>             // SD library for handling file operations on SD cards
#include <Update.h>         // Library for handling firmware updates over-the-air (OTA)

// WiFi network credentials
const char *ssid = "WiFi_NAME";
const char *password = "PASSWORD";

// Server information for checking and downloading firmware updates
const char *updateURL = "http://ota-demo-74718.web.app/firmware.bin";
// example: http://example.com/firmware-update?uuid=uuid&version=0.0.1

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

    WiFiClient client;
    HTTPClient http;

    if (http.begin(client, updateURL)) {    // Start HTTP connection
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_ALREADY_REPORTED) {
            Serial.println("Firmware already up to date.");

        } else if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
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
                WiFiClient *stream = http.getStreamPtr();
                size_t bufferSize = 1024;
                uint8_t buffer[bufferSize];    // Define a buffer for incoming data
                int bytesRead = stream->readBytes(buffer, bufferSize);
                while (bytesRead > 0) {    // Read and write the binary file
                    file.write(buffer, bytesRead);
                    bytesRead = stream->readBytes(buffer, bufferSize);
                }
                file.close();    // Close the file
                Serial.println("File saved successfully.");
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
