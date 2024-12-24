#ifndef MOTION_H
#define MOTION_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "display.h"

extern Adafruit_MPU6050 mpu;
extern sensors_event_t accel, gyro, temp;

// Data structure to hold orientation data
struct OrientationData {
    float pitch;
    float roll;
};

// Data structure to hold shake and orientation states
struct MotionStates {
    bool isShaking;
    bool isTilting;
    OrientationData orientation;
};

#define SHAKE_THRESHOLD 15
#define TILT_THRESHOLD 25.00

// Function prototypes
void initializeMPU6050(void);
void captureMPUData(void);
void gyroCalibration(int calibration_time);
void getGyro(float &calGyroX, float &calGyroY, float &calGyroZ, float gyroX, float gyroY, float gyroZ);

MotionStates detectMotionStates(float accelX, float accelY, float accelZ,
                                               float gyroX, float gyroY, float gyroZ,
                                               int shakeThreshold, float tiltThreshold);


#endif