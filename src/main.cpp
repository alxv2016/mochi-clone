#include <Arduino.h>
#include "display.h"
#include "animate.h"
#include "motion.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MPU6050");
  initializeOLED();
  initializeMPU6050();
  initializeGIFContext();
  displayBootMessage("ALXV");
  delay(2000);
}

void loop() {
  initializeEmotes();
  delay(1000);
}