#include "Inkplate.h"              // Include Inkplate library to the sketch
#include "driver/rtc_io.h"         // ESP32 library used for deep sleep and RTC wake up pins
#include <WiFi.h>                  // Library to connect to WiFi
#include <HTTPClient.h>            // HTTP library for calling API
#include <ArduinoJson.h>           // JSON parsing library
#include "SdFat.h"                 // Used to load secrets
#include <algorithm>               // For std::find

/*
 Message to future self: tried to implemement this where wrote to disk and saved images rather than
 making calls to server every sleep cycle (to save batter). Couldn't get permission to write to SD 
 so reverted to calling server.
*/

Inkplate display(INKPLATE_3BIT);   // Set library to work in gray mode

#define uS_TO_S_FACTOR 1000000     // Conversion factor for microseconds to seconds
unsigned long TIME_TO_SLEEP = 600; // Deep sleep time (in seconds); overwritten by API
#define API_URL "https://lindsayrgwatt.com/api/device-images/1/"

RTC_DATA_ATTR unsigned long bootCount = 0;    // Track the number of boots
RTC_DATA_ATTR std::vector<int> shownImages;   // Track indices of shown images

SdFat SD;

String ssid = "Nacho Wifi";
String password;
String apiKey;

// API data structure
struct APIData {
  int displayTime;
  std::vector<String> imageUrls;
};

// Function declarations
void connectWiFi();
bool readSecrets();
APIData fetchAPIData();
void downloadAndDisplayImage(String url);
void displayRandomImage(const APIData& apiData);

void setup() {
    Serial.begin(115200);
    Serial.print("Boot count: ");
    Serial.println(bootCount);

    display.begin();
    display.clearDisplay();
    display.setRotation(3);

    // Load the secret key from the filesystem
    if (!readSecrets()) {
        Serial.println("Failed to read secrets. Restarting...");
        return;
    }

    // Connect to Wi-Fi
    connectWiFi();        

    // Fetch API data
    APIData apiData = fetchAPIData();

    // Update TIME_TO_SLEEP based on display time from API (60 * displayTime)
    TIME_TO_SLEEP = 60 * apiData.displayTime;

    // Display a random image from the API
    displayRandomImage(apiData);

    // Sleep mode
    rtc_gpio_isolate(GPIO_NUM_12);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop() {
  // Nothing here as the device enters deep sleep after setup
}

// Function to connect to Wi-Fi
void connectWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

// Read the secret keys from the device filesystem
bool readSecrets() {
    // Ensure the SD card is initialized before attempting to open files
    if (!display.sdCardInit()) {
        Serial.println("Failed to initialize SD card.");
        return false;
    }

    SdFile file;  // Declare the SdFile object

    // Open the file in read-only mode
    if (!file.open("/secrets.txt", O_RDONLY)) {
        Serial.println("Failed to open secrets file.");
        display.println("File open error");
        display.display();
        display.sdCardSleep();
        return false;
    }

    // Buffer to store each line
    char line[100];  // Adjust the buffer size as needed

    // Read the file line by line
    int index = 0;
    while (file.available()) {
        char c = file.read();

        // Debugging: Print every character being read
        Serial.print("Read character: ");
        Serial.println(c, HEX);  // Print the ASCII value in hexadecimal

        // Detect line breaks and ensure all types are handled (\n and \r)
        if (c == '\n' || c == '\r') {
            // If we hit a line break, and there are characters in the buffer, process the line
            if (index > 0) {
                line[index] = '\0';  // Null-terminate the string

                // Process the line
                String strLine = String(line);
                strLine.trim();  // Remove any leftover newline or carriage return characters

                // Debugging: Print the read line
                Serial.print("Processed line: ");
                Serial.println(strLine);

                // Extract password and API key from the lines
                if (strLine.startsWith("password=")) {
                    password = strLine.substring(9);  // Get the password
                    Serial.print("Extracted password: ");
                    Serial.println(password);
                } else if (strLine.startsWith("api_key=")) {
                    apiKey = strLine.substring(8);    // Get the API key
                    Serial.print("Extracted API key: ");
                    Serial.println(apiKey);
                }

                // Reset the buffer for the next line
                index = 0;
            }
        } else {
            // Add characters to the buffer, but ensure it does not overflow
            if (index < sizeof(line) - 1) {
                line[index++] = c;
            }
        }
    }

    // **Handle the case where there's no newline after the last line**
    if (index > 0) {
        line[index] = '\0';  // Null-terminate the string

        // Process the final line
        String strLine = String(line);
        strLine.trim();

        Serial.print("Final processed line: ");
        Serial.println(strLine);

        if (strLine.startsWith("password=")) {
            password = strLine.substring(9);
            Serial.print("Extracted password: ");
            Serial.println(password);
        } else if (strLine.startsWith("api_key=")) {
            apiKey = strLine.substring(8);
            Serial.print("Extracted API key: ");
            Serial.println(apiKey);
        }
    }

    // Close the file after reading
    file.close();

    // Debugging: Check if password and API key were successfully extracted
    Serial.print("Final password: ");
    Serial.println(password);
    Serial.print("Final API key: ");
    Serial.println(apiKey);

    // Ensure that both the password and API key were read successfully
    if (password.length() == 0 || apiKey.length() == 0) {
        Serial.println("Failed to read all secrets.");
        return false;
    }

    return true;
}

// Fetch API data and parse JSON
APIData fetchAPIData() {
    APIData apiData;
    HTTPClient http;
    String url = String(API_URL) + apiKey + "/";

    // Print the URL being requested
    Serial.print("Making API request to: ");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpCode);

        String payload = http.getString();

        Serial.println("API Response:");
        Serial.println(payload);

        StaticJsonDocument<1024> doc;
        deserializeJson(doc, payload);

        apiData.displayTime = doc["display_time"];
        JsonArray images = doc["images"];

        for (JsonVariant v : images) {
            apiData.imageUrls.push_back(v.as<String>());
        }
    } else {
        Serial.println("Error on HTTP request");
    }

    http.end();
    return apiData;
}

// Download and display an image from a URL
void downloadAndDisplayImage(String url) {
    Serial.println("Downloading and displaying image: " + url);

    // Use the drawImage method directly for a JPEG image
    int result = display.drawImage(url.c_str(), display.JPG, 0, 0);

    if (result != 1) {
        Serial.print("Failed to display image with error code: ");
        Serial.println(result);
        Serial.println("URL: " + url);
    } else {
        display.display();  // Refresh the screen after the image is drawn
        Serial.println("Displayed image from URL: " + url);
    }
}

// Display a random image from the API response, ensuring no repeats until all are shown
void displayRandomImage(const APIData& apiData) {
    if (apiData.imageUrls.empty()) {
        Serial.println("No images available from the API.");
        return;
    }

    // Check if we've shown all images, if so, reset the shownImages list
    if (shownImages.size() == apiData.imageUrls.size()) {
        Serial.println("All images have been shown. Resetting list.");
        shownImages.clear();
    }

    // Pick an image that hasn't been shown yet
    int index;
    do {
        index = random(apiData.imageUrls.size());
    } while (std::find(shownImages.begin(), shownImages.end(), index) != shownImages.end());

    // Add the chosen image index to the shownImages list
    shownImages.push_back(index);

    // Display the chosen image
    String imageUrl = apiData.imageUrls[index];
    Serial.println("Displaying image: " + imageUrl);
    downloadAndDisplayImage(imageUrl);
}