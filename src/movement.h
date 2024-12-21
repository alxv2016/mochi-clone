#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "display.h"

// MPU PINS
#define SDA_PIN 11
#define SCL_PIN 12

extern Adafruit_MPU6050 mpu;
// Struct to hold position and movement values
struct PositionData {
  int logicalX;
  int logicalY;
  int deltaX;
  int deltaY;
};

struct OrientationData {
  float yaw;
  float pitch;
  float roll;
};

enum ShakeLevel {
  NO_SHAKE = 0,
  LIGHT_SHAKE = 1,
  MEDIUM_SHAKE = 2,
  HARD_SHAKE = 3
};

// Thresholds for shake intensity
#define LIGHT_SHAKE_THRESHOLD 2.0
#define MEDIUM_SHAKE_THRESHOLD 5.0
#define HARD_SHAKE_THRESHOLD 8.0

// Movement scaling
#define MOVEMENT_SCALING 5 


// Function prototypes
OrientationData calculateOrientation(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ, float dt);
ShakeLevel detectShake(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ);
PositionData updateElementPosition(float accelX, float accelY, float gyroX, float gyroY, float dt);
void initializeMPU6050(void);


#endif