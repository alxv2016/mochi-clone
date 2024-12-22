#include "movement.h"

Adafruit_MPU6050 mpu;
bool isShaking = false; // State variable to track shaking

void initializeMPU6050() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  
}

// Function to calculate yaw, pitch, and roll
OrientationData calculateOrientation(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ, float dt) {
  static float tilt = 0.0;
  static float rotate = 0.0;
  static float turn = 0.0;

  // Calculate pitch and roll from accelerometer data
  float accelPitch = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180 / M_PI;
  float accelRoll = atan2(-accelX, accelZ) * 180 / M_PI;

  // Integrate gyroscope data to get angles
  tilt += gyroX * dt;
  rotate += gyroY * dt;
  turn += gyroZ * dt;

  // Complementary filter to combine accelerometer and gyroscope data
  const float alpha = 0.98;
  tilt = alpha * tilt + (1 - alpha) * accelPitch;
  rotate = alpha * rotate + (1 - alpha) * accelRoll;

  OrientationData orientation = {turn, tilt, rotate};
  return orientation;
}

bool detectShake(
  float accelX, float accelY, float accelZ, 
  float gyroX, float gyroY, float gyroZ
) {
  // Calculate the magnitude of the accelerometer data
  float accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  float dynamicAccelMagnitude = abs(accelMagnitude - 9.8); // Subtract gravity

  // Calculate the magnitude of the gyroscope data
  float gyroMagnitude = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));

  // Combine accelerometer and gyroscope data
  float combinedMagnitude = dynamicAccelMagnitude + gyroMagnitude;

  // Determine if shaking
  if (combinedMagnitude > SHAKE_THRESHOLD) {
    if (!isShaking) {
      isShaking = true; // Transition to "shaking" state
      return true;      // Trigger shake detection
    }
    // Already shaking, do nothing
    return false;
  } else {
    isShaking = false; // Reset state when not shaking
    return false;
  }
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