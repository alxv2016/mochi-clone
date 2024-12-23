#include "animate.h"
#include "display.h"
#include "movement.h"
#include <Arduino.h>

void logMPUData() {
  OrientationData orientation = calculateOrientation(
      accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
      gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, 0.01);

  Serial.print("Orientation - turn: ");
  Serial.print(orientation.turn);
  Serial.print("°, tilt: ");
  Serial.print(orientation.tilt);
  Serial.print("°, Rotate: ");
  Serial.println(orientation.rotate);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MPU6050");
  initializeOLED();
  initializeMPU6050();
  // initializeGIF();
  displayBootMessage("ALXV");
  delay(3000);
}

void loop() {
  // Handle GIF interaction based on shaking
  interactRandomGIF();
  // Allow other tasks to run
  delay(1000); // Optional small delay to avoid excessive CPU usage
}