#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

extern Adafruit_MPU6050 mpu;
extern sensors_event_t accel, gyro, temp;
extern bool isShaking; // State variable to track shaking
// Struct to hold position and movement values
struct PositionData {
  int logicalX;
  int logicalY;
  int deltaX;
  int deltaY;
};

struct OrientationData {
  float turn;
  float tilt;
  float rotate;
};

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128
// Movement scaling
#define MOVEMENT_SCALING 5
#define SHAKE_THRESHOLD 4

// Function prototypes
OrientationData calculateOrientation(float accelX, float accelY, float accelZ,
                                     float gyroX, float gyroY, float gyroZ,
                                     float dt);
bool detectShake(float accelX, float accelY, float accelZ, float gyroX,
                 float gyroY, float gyroZ);
PositionData updateElementPosition(float accelX, float accelY, float gyroX,
                                   float gyroY, float dt);
void initializeMPU6050(void);
void captureMPUData(void);

#endif