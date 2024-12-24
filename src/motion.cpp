#include "motion.h"

Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temp;
bool mpuInitialized = false;

unsigned long lastCaptureTime = 0; // Variable to store the last capture time
const unsigned long debounceDelay = 1000; // 500 milliseconds debounce delay

void initializeMPU6050() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(
      true); // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
  mpuInitialized = true;
  Serial.println("MPU6050 Found!");
}
void captureMPUData() {
  if (!mpuInitialized) {
    Serial.println("Error: MPU not initialized.");
    return;
  }

  if (mpu.getMotionInterruptStatus()) {
    unsigned long currentTime = millis();
    if (currentTime - lastCaptureTime >= debounceDelay) {
      mpu.getEvent(&accel, &gyro, &temp);
      lastCaptureTime = currentTime; // Update the last capture time
    }
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

OrientationData calculateOrientation(float accelX, float accelY, float accelZ,
                                     float gyroX, float gyroY, float gyroZ,
                                     float dt) {
  // Calculate pitch and roll from accelerometer data
  float pitch =
      atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180.0 / M_PI;
  float roll =
      atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / M_PI;

  // Apply absolute value and round to the nearest whole number
  pitch = round(abs(pitch));
  roll = round(abs(roll));

  // Log readings
  // Serial.print("Pitch: ");
  // Serial.print(pitch);
  // Serial.print(", Roll: ");
  // Serial.println(roll);

  // Serial.print("Acceleration X: ");
  // Serial.print(accelX);
  // Serial.print(", Y: ");
  // Serial.print(accelY);
  // Serial.print(", Z: ");
  // Serial.print(accelZ);
  // Serial.println(" m/s^2");

  // Serial.print("Rotation X: ");
  // Serial.print(gyroX);
  // Serial.print(", Y: ");
  // Serial.print(gyroY);
  // Serial.print(", Z: ");
  // Serial.print(gyroZ);
  // Serial.println(" rad/s");

  return {pitch, roll};
}

// General function to detect a state (e.g., shaking or turning)
bool detectState(float magnitude, float threshold, bool &currentState,
                 unsigned long &lastStateTime, const char *stateName,
                 const char *stopMessage) {

  const unsigned long RESET_TIMEOUT = debounceDelay;

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
                                               float turnThreshold,
                                               float tiltThreshold) {

  static bool isShaking = false;
  static bool isTilting = false;
  static unsigned long lastShakeTime = 0;
  static unsigned long lastTiltTime = 0;

  const unsigned long RESET_TIMEOUT = 2000;
  unsigned long currentTime = millis();

  // Calculate combined magnitude
  int combinedMagnitude =
      calculateCombinedMagnitude(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);
  // Detect shake state with debounce
  if (currentTime - lastShakeTime >= debounceDelay) {
    if (detectState(combinedMagnitude, shakeThreshold, isShaking, lastShakeTime,
                    "Shaking", "Shaking stopped.")) {
      lastShakeTime = currentTime;
    }
  }
  detectNoState(isShaking, lastShakeTime, RESET_TIMEOUT, "Shaking stopped.");

  // Calculate orientation
  OrientationData orientation =
      calculateOrientation(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, dt);

  // Detect tilt state with debounce
  if (currentTime - lastTiltTime >= debounceDelay) {
    if (detectState(abs(orientation.pitch), tiltThreshold, isTilting,
                    lastTiltTime, "Tilt", "Tilt stopped.")) {
      lastTiltTime = currentTime;
    }
  }
  detectNoState(isTilting, lastTiltTime, RESET_TIMEOUT, "Tilt stopped.");

  return {isShaking, isTilting, orientation};
}