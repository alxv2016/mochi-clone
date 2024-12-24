#include "movement.h"

Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temp;
bool mpuInitialized = false;

unsigned long lastCaptureTime = 0; // Variable to store the last capture time
const unsigned long captureInterval = 100; // Interval in milliseconds

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
  unsigned long currentTime = millis();
  if (currentTime - lastCaptureTime >= captureInterval) {
    mpu.getEvent(&accel, &gyro, &temp);
    lastCaptureTime = currentTime; // Update the last capture time
  }
}

int calculateCombinedMagnitude(float accelX, float accelY, float accelZ,
                               float gyroX, float gyroY, float gyroZ) {
  const float GRAVITY = 9.8;
  float accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  float dynamicAccelMagnitude = abs(accelMagnitude - GRAVITY);
  float gyroMagnitude = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));
  return (int)round(dynamicAccelMagnitude + gyroMagnitude);
}

// Function to calculate orientation using accelerometer and gyroscope data
OrientationData calculateOrientation(float accelX, float accelY, float accelZ,
                                     float gyroX, float gyroY, float gyroZ,
                                     float dt) {
  static float tilt = 0.0;
  static float rotate = 0.0;
  static float turn = 0.0;

  float accelPitch =
      atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180 / M_PI;
  float accelRoll = atan2(-accelX, accelZ) * 180 / M_PI;

  tilt += gyroX * dt;
  rotate += gyroY * dt;
  turn += gyroZ * dt;

  float turnDegrees = turn * 180 / M_PI;

  const float alpha = 0.98;
  tilt = alpha * tilt + (1 - alpha) * accelPitch;
  rotate = alpha * rotate + (1 - alpha) * accelRoll;

  int roundedTurn = round(turnDegrees);
  int roundedTilt = round(tilt);
  int roundedRotate = round(rotate);

  return {roundedTurn, roundedTilt, roundedRotate};
}

// General function to detect a state (e.g., shaking or turning)
bool detectState(float magnitude, float threshold, bool &currentState,
                 unsigned long &lastStateTime, const char *stateName,
                 const char *stopMessage) {
                  
  const unsigned long RESET_TIMEOUT = REST_TIMEOUT;

  if (magnitude >= threshold) {
    if (!currentState) {
      currentState = true;
      lastStateTime = millis();
      Serial.print(stateName);
      Serial.println(" detected!");
    } else {
      lastStateTime = millis();
    }
    return true;
  } else if (currentState && millis() - lastStateTime > RESET_TIMEOUT) {
    currentState = false;
    Serial.println(stopMessage);
  }
  return false;
}

// General function to detect when a state (e.g., shaking or turning) has
// stopped
bool detectNoState(bool &currentState, unsigned long lastStateTime,
                   unsigned long timeout, const char *stopMessage) {
  if (currentState && millis() - lastStateTime > timeout) {
    currentState = false;
    Serial.println(stopMessage);
    return true;
  }
  return false;
}

// Main function to detect both shake and orientation states
ShakeOrientationData detectShakeAndOrientation(float accelX, float accelY,
                                               float accelZ, float gyroX,
                                               float gyroY, float gyroZ,
                                               float dt, int shakeThreshold,
                                               float turnThreshold, float tiltThreshold) {

  static bool isShaking = false;
  static bool isTurning = false;
  static bool isTilting = false;
  static unsigned long lastShakeTime = 0;
  static unsigned long lastTurnTime = 0;
  static unsigned long lastTiltTime = 0;

  const unsigned long RESET_TIMEOUT = RESET_TIMEOUT;

  // Calculate combined magnitude
  int combinedMagnitude = calculateCombinedMagnitude(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);

  // Detect shake state
  detectState(combinedMagnitude, shakeThreshold, isShaking, lastShakeTime, "Shaking", "Shaking stopped.");
  detectNoState(isShaking, lastShakeTime, RESET_TIMEOUT, "Shaking stopped.");

  // Calculate orientation
  OrientationData orientation =
      calculateOrientation(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, dt);

  // Detect turn state
  detectState(abs(orientation.turn), turnThreshold, isTurning, lastTurnTime, "Turn", "Turn stopped.");
  detectNoState(isTurning, lastTurnTime, RESET_TIMEOUT, "Turn stopped.");

  // Detect tilt state
  detectState(abs(orientation.tilt), tiltThreshold, isTilting, lastTiltTime, "Tilt", "Tilt stopped.");
  detectNoState(isTilting, lastTiltTime, RESET_TIMEOUT, "Tilt stopped.");

  return {isShaking, isTurning, isTilting, orientation};
}