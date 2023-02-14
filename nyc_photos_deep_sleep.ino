#include "Inkplate.h" //Include Inkplate library to the sketch
#include "driver/rtc_io.h" //ESP32 library used for deep sleep and RTC wake up pins

Inkplate display(INKPLATE_3BIT); // Create object on Inkplate library and set library to work in gray mode (3-bit)
                                 // Other option is BW mode, which is demonstrated in next example
                                 // "Inkplate_basic_BW"

#define uS_TO_S_FACTOR 1000000 // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  600      // How long ESP32 will be in deep sleep (in seconds)

RTC_DATA_ATTR int imageCount = 300; // Expect this many images on SD card and each has integer as name (e.g., '3.jpg' or similar)
RTC_DATA_ATTR int counter = 1;
RTC_DATA_ATTR int lastImage = 0;
RTC_DATA_ATTR long randNumber;

void setup() {
    // Generic setup
    Serial.begin(115200);
  
    display.begin();        // Init library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear any data that may have been in (software) frame buffer.
                            //(NOTE! This does not clean image on screen, it only clears it in the frame buffer inside
                            // ESP32).
    display.display();      // Clear everything that has previously been on a screen
    
    display.setRotation(3);

    // Main program - but running in setup due to low power mode
    randNumber = random(imageCount);

    if (randNumber != lastImage) {
      Serial.println (randNumber);
      // Draw from SD card
      if (display.sdCardInit())
      {
          String path = String(randNumber) + ".jpg";
  
          if (!display.drawImage(path, 0, 0, 1))
          {
              display.println("Image open error");
              display.display();  
          } else {
              // Put this here to defend against invalid file names
              display.display();
              lastImage = randNumber;
          }
      }
      else
      {
          // If SD card init not success, display error on screen and stop the program (using infinite loop)
          display.println("SD Card error!");
          display.partialUpdate();
          while (true)
              ;
      }
    }

    // Sleep mode
    rtc_gpio_isolate(GPIO_NUM_12); // Isolate/disable GPIO12 on ESP32 (only to reduce power consumption in sleep)
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // Activate wake-up timer -- wake up after 20s here
    esp_deep_sleep_start();                                        // Put ESP32 into deep sleep. Program stops here.
}

void loop() {
}
