#include "motion.h"

Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temp;
bool mpuInitialized = false;

unsigned long lastCaptureTime = 0; // Variable to store the last capture time
const unsigned long debounceDelay = 500; // 500 milliseconds debounce delay
// Gyroscope offsets
float offset_gx = 0.0, offset_gy = 0.0, offset_gz = 0.0;

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
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 Found!");

  // Wait for MPU6050 to settle
  int settling_time = 4;
  Serial.printf("Settling MPU for %d seconds...\n", settling_time);
  delay(settling_time * 1000);
  Serial.println("MPU is done settling.");
  mpuInitialized = true;
}

// Function to get gyroscope readings
void getGyro(float &calGyroX, float &calGyroY, float &calGyroZ, float gyroX, float gyroY, float gyroZ) {
  // Get gyroscope readings and apply offsets
  calGyroX = gyroX * 57.2958 - offset_gx; // Convert radians/sec to degrees/sec
  calGyroY = gyroY * 57.2958 - offset_gy;
  calGyroZ = gyroZ * 57.2958 - offset_gz;
}

// Function to calibrate the gyroscope
void gyroCalibration(int calibration_time) {
  int num_samples = 0;
  float sum_gx = 0.0, sum_gy = 0.0, sum_gz = 0.0;

  unsigned long start_time = millis();
  while (millis() - start_time < calibration_time * 1000) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    // Accumulate raw gyroscope readings
    sum_gx += gyro.gyro.x * 57.2958; // Convert radians/sec to degrees/sec
    sum_gy += gyro.gyro.y * 57.2958;
    sum_gz += gyro.gyro.z * 57.2958;

    num_samples++;
    if (num_samples % 100 == 0) {
      Serial.printf("Still calibrating... %d samples collected\n", num_samples);
    }

    delay(10); // Adjust based on MPU6050 read rate
  }

  // Calculate and store offsets
  offset_gx = sum_gx / num_samples;
  offset_gy = sum_gy / num_samples;
  offset_gz = sum_gz / num_samples;

  Serial.printf("Gyroscope calibration complete!\nOffsets: X = %.2f, Y = %.2f, Z = %.2f\n",
                offset_gx, offset_gy, offset_gz);
}

void captureMPUData() {
  if (!mpuInitialized) {
    Serial.println("Error: MPU not initialized.");
    return;
  }
  unsigned long currentTime = millis();
  if (currentTime - lastCaptureTime >= debounceDelay) {
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