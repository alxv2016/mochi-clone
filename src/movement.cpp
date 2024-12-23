#include "movement.h"

Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temp;
bool isShaking = false; // State variable to track shaking
bool mpuInitialized = false; 

void initializeMPU6050() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpuInitialized = true; 
  Serial.println("MPU6050 Found!");
}
void captureMPUData() {
  if (!mpuInitialized) {
    Serial.println("Error: MPU not initialized.");
    return;
  }
  mpu.getEvent(&accel, &gyro, &temp);
}

static bool handleShakeDetected(int lastShakeTime) {
  if (!isShaking) {
    isShaking = true;
    lastShakeTime = millis();
    Serial.println("Shaking detected!");
    return true;
  }
  lastShakeTime = millis();
  return false;
}

static bool handleNoShakeDetected(int lastShakeTime, int resetTime) {
  if (isShaking && millis() - lastShakeTime > resetTime) {
    isShaking = false;
    Serial.println("Shaking stopped.");
  }
  return false;
}

static int calculateCombinedMagnitude(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ) {
  const float GRAVITY = 9.8; // Slightly lower gravity constant for increased sensitivity.
  // const float GYRO_WEIGHT = 1.2; // Increase the weight of the gyroscope data.
  // Calculate the magnitude of acceleration by combining all three axes using
  // the Pythagorean theorem.
  float accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  // Remove the constant effect of gravity to focus on dynamic movement.
  float dynamicAccelMagnitude = abs(accelMagnitude - GRAVITY);
  // Calculate the magnitude of the gyroscope data (rotational movement).
  float gyroMagnitude = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));
  int combinedMagnitude = (int)round(dynamicAccelMagnitude + gyroMagnitude);
  return combinedMagnitude;
}

bool detectShake(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ) {
  // Static variables retain their values between function calls.
  // Stores the time (in milliseconds) when shaking was last detected.
  const int shakeThreshold = SHAKE_THRESHOLD;
  // Time (in ms) to wait before resetting the "shaking" state.
  const unsigned long RESET_TIMEOUT = 2000;
  const unsigned long lastShakeTime = 0;
  // Keeps track of whether the device is currently shaking.
  bool isShaking = false;
  // Constants make it easier to adjust values without digging into the code.
  int combinedMagnitude = calculateCombinedMagnitude(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);

  // Log the calculated combined magnitude to the serial monitor for debugging.
  // Serial.print("Shake Value (Combined Magnitude): ");
  // Serial.println(combinedMagnitude);

  if (combinedMagnitude >= shakeThreshold) {
    return handleShakeDetected(lastShakeTime);
  } else {
    return handleNoShakeDetected(lastShakeTime, RESET_TIMEOUT);
  }

}

// Function to calculate yaw, pitch, and roll
OrientationData calculateOrientation(float accelX, float accelY, float accelZ,
                                     float gyroX, float gyroY, float gyroZ,
                                     float dt) {
  static float tilt = 0.0;
  static float rotate = 0.0;
  static float turn = 0.0;

  // Calculate pitch and roll from accelerometer data
  float accelPitch =
      atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180 / M_PI;
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

PositionData updateElementPosition(float accelX, float accelY, float gyroX,
                                   float gyroY, float dt) {
  // Static variables to retain their values between function calls
  static int elementX = DISPLAY_WIDTH / 2;  // Initial X position (centered)
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