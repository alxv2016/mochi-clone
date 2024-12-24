#include <Arduino.h>
#include "animate.h"
#include "display.h"
#include "motion.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MPU6050");
  initializeOLED();
  initializeMPU6050();
  initializeGIF();
  displayBootMessage("ALXV");
  delay(3000);
}

void loop() {
  // Handle GIF interaction based on shaking
  interactRandomGIF();
  // Allow other tasks to run
  delay(1000); // Optional small delay to avoid excessive CPU usage
}