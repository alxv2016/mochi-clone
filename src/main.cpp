#include <Arduino.h>
// #include "animate.h"
#include "display.h"
#include "movement.h"

void logMPUData() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  ShakeLevel shake = detectShake(accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, gyro.gyro.x, gyro.gyro.y, gyro.gyro.z);

  switch (shake) {
    case HARD_SHAKE:
      Serial.println("Hard Shake Detected!");
      break;
    case MEDIUM_SHAKE:
      Serial.println("Medium Shake Detected!");
      break;
    case LIGHT_SHAKE:
      Serial.println("Light Shake Detected!");
      break;
    case NO_SHAKE:
    default:
      // No shake detected
      break;
  }

    // Update element position and retrieve the position data
  PositionData position = updateElementPosition(accel.acceleration.x, accel.acceleration.y, gyro.gyro.x, gyro.gyro.y, 0.01);

  // Log the position data
  Serial.print("Logical Position - X: ");
  Serial.print(position.logicalX);
  Serial.print(", Y: ");
  Serial.println(position.logicalY);

  Serial.print("Movement Applied - Delta X: ");
  Serial.print(position.deltaX);
  Serial.print(", Delta Y: ");
  Serial.println(position.deltaY);

  OrientationData orientation = calculateOrientation(accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, 0.01);

  Serial.print("Orientation - Yaw: ");
  Serial.print(orientation.yaw);
  Serial.print("°, Pitch: ");
  Serial.print(orientation.pitch);
  Serial.print("°, Roll: ");
  Serial.println(orientation.roll);
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Give serial time to start
  initializeOLED();
  displayBootMessage("ALXV");
  initializeMPU6050();
  // initializeGIF();
}

void loop() {
  Serial.println("Hello world");
  
  // playRandomGIF();
  logMPUData();
  delay(100); // Adjust delay as needed for responsiveness
}