#ifndef MOTION_H
#define MOTION_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

extern Adafruit_MPU6050 mpu;
extern sensors_event_t accel, gyro, temp;

// Data structure to hold orientation data
struct OrientationData {
    float pitch;
    float roll;
};

// Data structure to hold shake and orientation states
struct ShakeOrientationData {
    bool isShaking;
    bool isTilting;
    OrientationData orientation;
};

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128
// Movement scaling
#define MOVEMENT_SCALING 5
#define SHAKE_THRESHOLD 15
#define TURN_THRESHOLD 25.00
#define TILT_THRESHOLD 25.00

// Function prototypes
void initializeMPU6050(void);
void captureMPUData(void);
ShakeOrientationData detectShakeAndOrientation(float accelX, float accelY, float accelZ,
                                               float gyroX, float gyroY, float gyroZ,
                                               float dt, int shakeThreshold, float turnThreshold, float tiltThreshold);


#endif