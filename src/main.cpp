#include <Arduino.h>
#include "display.h"
#include "movement.h"
#include "animate.h"

void logMPUData() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  if (detectShake(accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, gyro.gyro.x, gyro.gyro.y, gyro.gyro.z)) {
    Serial.println("Shake detected!");
  } else {
    Serial.println("No shake detected. Already shaking");
  }

  OrientationData orientation = calculateOrientation(accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, 0.01);

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
  initializeGIF();
  displayBootMessage("ALXV");
  delay(3000);
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("MPU6050 Data:");
  logMPUData();
  playRandomGIF();
  delay(100);
}