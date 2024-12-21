#include <Arduino.h>
// #include "animate.h"
#include "display.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// // Define your custom I2C pins
// #define SDA_PIN 8
// #define SCL_PIN 3
// // Thresholds for shake intensity
// #define LIGHT_SHAKE_THRESHOLD 2.0 // Adjust based on testing
// #define MEDIUM_SHAKE_THRESHOLD 5.0
// #define HARD_SHAKE_THRESHOLD 8.0

// Adafruit_MPU6050 mpu;

// Variable to store the last detected shake intensity
// String lastShakeMessage = "";

void setup() {
  Serial.begin(115200);
  delay(2000); // Give serial time to start
  // // Initialize Wire with custom pins
  // Wire.begin(SDA_PIN, SCL_PIN);
  // // Initialize the MPU6050 with the customized Wire instance
  // if (!mpu.begin(0x68, &Wire)) { // 0x68 is the default I2C address for the
  // MPU6050
  //   Serial.println("Failed to find MPU6050 chip");
  //   while (1) {
  //     delay(10);
  //   }
  // }

  // Serial.println("MPU6050 Found!");

  initializeOLED();
  displayBootMessage("ALXV");
  // initializeGIF();
}

void loop() {
  Serial.println("Hello world");
  //   // playRandomGIF();
  //   sensors_event_t accel, gyro, temp;
  //   mpu.getEvent(&accel, &gyro, &temp);

  //   // Calculate the magnitude of acceleration
  //   float magnitude = sqrt(sq(accel.acceleration.x) +
  //                          sq(accel.acceleration.y) +
  //                          sq(accel.acceleration.z));

  //   // Remove gravity (approximately 9.8 m/s²)
  //   float dynamicMagnitude = abs(magnitude - 9.8);

  //  // Determine shake intensity
  //   String shakeMessage = "";
  //   if (dynamicMagnitude > HARD_SHAKE_THRESHOLD) {
  //     shakeMessage = "Hard Shake Detected!";
  //   } else if (dynamicMagnitude > MEDIUM_SHAKE_THRESHOLD) {
  //     shakeMessage = "Medium Shake Detected!";
  //   } else if (dynamicMagnitude > LIGHT_SHAKE_THRESHOLD) {
  //     shakeMessage = "Light Shake Detected!";
  //   }

  //   // Update OLED only if the message changes
  //   if (shakeMessage != lastShakeMessage && shakeMessage != "") {
  //     lastShakeMessage = shakeMessage;

  //     // Clear the area where the text will appear
  //     oled.fillRect(0, 0, DISPLAY_WIDTH, 20, TFT_BLACK);

  //     // Display the shake message
  //     oled.setCursor(2, 2); // Adjust the position as needed
  //     oled.setTextColor(TFT_WHITE);
  //     oled.println(shakeMessage);

  //     // Print to Serial Monitor (optional for debugging)
  //     Serial.println(shakeMessage);
  //   }

  // playRandomGIF();
  delay(50); // Adjust delay as needed for responsiveness
}