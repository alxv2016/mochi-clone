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
  float accelX, float accelY, float accelZ,   // Accelerometer data for X, Y, Z axes
  float gyroX, float gyroY, float gyroZ      // Gyroscope data for X, Y, Z axes
) {
  // Static variables retain their values between function calls.
  static unsigned long lastShakeTime = 0;    // Stores the time (in milliseconds) when shaking was last detected.
  static bool isShaking = false;             // Keeps track of whether the device is currently shaking.

  // Constants make it easier to adjust values without digging into the code.
  const unsigned long RESET_TIMEOUT = 1000; // Time (in ms) to wait before resetting the "shaking" state.
  const float GRAVITY = 9.8;                 // Approximate acceleration due to gravity (m/s²).

  // Calculate the magnitude of acceleration by combining all three axes using the Pythagorean theorem.
  float accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));

  // Remove the constant effect of gravity to focus on dynamic movement.
  float dynamicAccelMagnitude = abs(accelMagnitude - GRAVITY);

  // Calculate the magnitude of the gyroscope data (rotational movement).
  float gyroMagnitude = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));

  // Combine accelerometer and gyroscope data to get an overall movement value.
  float combinedMagnitude = dynamicAccelMagnitude + gyroMagnitude;

  // Log the calculated combined magnitude to the serial monitor for debugging.
  Serial.print("Shake Value (Combined Magnitude): ");
  Serial.println(combinedMagnitude);

  // Check if the combined magnitude exceeds the defined shake threshold.
  if (combinedMagnitude > SHAKE_THRESHOLD) {
    if (!isShaking) {                // If not already shaking:
      isShaking = true;              // Update the state to indicate shaking.
      lastShakeTime = millis();      // Record the current time for reset logic.
      
      // Log to the serial monitor that shaking has started.
      Serial.println("Shaking detected!");

      return true;                   // Return `true` to indicate shaking was detected.
    }

    // If already shaking, simply update the last shake time and return `false`.
    lastShakeTime = millis();        // Keeps the reset timeout from triggering.
    return false;                    // Return `false` because this is not a new shake event.
  }

  // If the combined magnitude is below the shake threshold:
  if (isShaking && millis() - lastShakeTime > RESET_TIMEOUT) {
    // If enough time has passed since the last shake:
    isShaking = false;               // Reset the "shaking" state to `false`.

    // Log to the serial monitor that shaking has stopped.
    Serial.println("Shaking stopped.");
  }

  return false;                      // Return `false` because no shaking was detected.
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