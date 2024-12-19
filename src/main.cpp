#include "screen-animate.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

// Define your custom I2C pins
#define SDA_PIN 15
#define SCL_PIN 16
// Thresholds for shake intensity
#define LIGHT_SHAKE_THRESHOLD 2.0 // Adjust based on testing
#define MEDIUM_SHAKE_THRESHOLD 5.0
#define HARD_SHAKE_THRESHOLD 8.0

Adafruit_MPU6050 mpu;

// Variable to store the last detected shake intensity
String lastShakeMessage = "";

void initializeOLED() {
  oled.begin(LCD_SSD1351, FLAGS_NONE, SPI_FREQUENCY, CS_PIN, DC_PIN, RST_PIN,
             -1, -1, MOSI_PIN, CLK_PIN);
  oled.setRotation(LCD_ORIENTATION_90);
  oled.allocBuffer();
  oled.fillScreen(TFT_BLACK);
  oled.setFont(FONT_8x8);
  oled.setTextColor(TFT_WHITE);
}

void displayBootMessage(const char *message) {
  int16_t x1, y1;
  uint16_t textWidth, textHeight;

  oled.getTextBounds(message, 0, 0, &x1, &y1, &textWidth, &textHeight);

  int16_t centeredX = (DISPLAY_WIDTH - textWidth) / 2;
  int16_t centeredY = (DISPLAY_HEIGHT - textHeight) / 2;

  oled.setCursor(centeredX, centeredY);
  oled.println(message);
}

void initializeGIF() { gif.begin(GIF_PALETTE_RGB565_BE); }

void setup() {
  // Initialize Wire with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  // Initialize the MPU6050 with the customized Wire instance
  if (!mpu.begin(0x68, &Wire)) { // 0x68 is the default I2C address for the MPU6050
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");

  initializeOLED();
 displayBootMessage("Shake Detector Ready");
  // Optional delay to display the boot message
  delay(2000);
  // initializeGIF();
}

void loop() {
  // playRandomGIF();
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate the magnitude of acceleration
  float magnitude = sqrt(sq(accel.acceleration.x) +
                         sq(accel.acceleration.y) +
                         sq(accel.acceleration.z));

  // Remove gravity (approximately 9.8 m/s²)
  float dynamicMagnitude = abs(magnitude - 9.8);

 // Determine shake intensity
  String shakeMessage = "";
  if (dynamicMagnitude > HARD_SHAKE_THRESHOLD) {
    shakeMessage = "Hard Shake Detected!";
  } else if (dynamicMagnitude > MEDIUM_SHAKE_THRESHOLD) {
    shakeMessage = "Medium Shake Detected!";
  } else if (dynamicMagnitude > LIGHT_SHAKE_THRESHOLD) {
    shakeMessage = "Light Shake Detected!";
  }

  // Update OLED only if the message changes
  if (shakeMessage != lastShakeMessage && shakeMessage != "") {
    lastShakeMessage = shakeMessage;

    // Clear the area where the text will appear
    oled.fillRect(0, 0, DISPLAY_WIDTH, 20, TFT_BLACK);

    // Display the shake message
    oled.setCursor(2, 2); // Adjust the position as needed
    oled.setTextColor(TFT_WHITE);
    oled.println(shakeMessage);
    oled.display();

    // Print to Serial Monitor (optional for debugging)
    Serial.println(shakeMessage);
  }

  delay(50); // Adjust delay as needed for responsiveness
}