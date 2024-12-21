#include "movement.h"

Adafruit_MPU6050 mpu;

void initializeMPU6050() {
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

// Function to calculate yaw, pitch, and roll
OrientationData calculateOrientation(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ, float dt) {
  static float pitch = 0.0;
  static float roll = 0.0;
  static float yaw = 0.0;

  // Calculate pitch and roll from accelerometer data
  float accelPitch = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180 / M_PI;
  float accelRoll = atan2(-accelX, accelZ) * 180 / M_PI;

  // Integrate gyroscope data to get angles
  pitch += gyroX * dt;
  roll += gyroY * dt;
  yaw += gyroZ * dt;

  // Complementary filter to combine accelerometer and gyroscope data
  const float alpha = 0.98;
  pitch = alpha * pitch + (1 - alpha) * accelPitch;
  roll = alpha * roll + (1 - alpha) * accelRoll;

  OrientationData orientation = {yaw, pitch, roll};
  return orientation;
}

ShakeLevel detectShake(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ) {
  float accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  float dynamicAccelMagnitude = abs(accelMagnitude - 9.8);

  float gyroMagnitude = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));

  // Combine accelerometer and gyroscope data
  float combinedMagnitude = dynamicAccelMagnitude + gyroMagnitude;

  if (combinedMagnitude > HARD_SHAKE_THRESHOLD) {
    return HARD_SHAKE;
  } else if (combinedMagnitude > MEDIUM_SHAKE_THRESHOLD) {
    return MEDIUM_SHAKE;
  } else if (combinedMagnitude > LIGHT_SHAKE_THRESHOLD) {
    return LIGHT_SHAKE;
  }

  return NO_SHAKE;
}

PositionData updateElementPosition(float accelX, float accelY, float gyroX, float gyroY, float dt) {
    // Static variables to retain their values between function calls
  static int elementX = DISPLAY_WIDTH / 2; // Initial X position (centered)
  static int elementY = DISPLAY_HEIGHT / 2; // Initial Y position (centered)
  // Scale the movement
  int deltaX = (accelX + gyroX * dt) * MOVEMENT_SCALING;
  int deltaY = (accelY + gyroY * dt) * MOVEMENT_SCALING;

  // Update the logical position
  elementX += deltaX;
  elementY += deltaY;

  // Clamp the position
  elementX = constrain(elementX, -DISPLAY_WIDTH + 10, DISPLAY_WIDTH - 10);
  elementY = constrain(elementY, -DISPLAY_HEIGHT + 10, DISPLAY_HEIGHT - 10);

  // Return a struct with the updated data
  PositionData position = {elementX, elementY, deltaX, deltaY};
  return position;
}