#ifndef LSM6DSOX_H
#define LSM6DSOX_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../utils/sensors_data.h"


#define LSM6DSOX_ADDRESS            0x6A

#define LSM6DSOX_WHO_AM_I           0X0F
#define LSM6DSOX_CTRL1_XL           0X10        // Accelerometer control register 1
#define LSM6DSOX_CTRL2_G            0X11        // Gyroscope control register 2
#define LSM6DSOX_STATUS_REG         0X1E        // Contain information if new data is available
// #define LSM6DSOX_CTRL6_C            0X15     // Accelerometer power mode
#define LSM6DSOX_CTRL7_G            0X16        // Control register 7
#define LSM6DSOX_CTRL8_XL           0X17        // Control register 8

#define LSM6DSOX_OUT_TEMP_L         0X20        // termometer data
#define LSM6DSOX_OUTX_L_G           0X22        // gyro data
#define LSM6DSOX_OUTX_L_A           0X28        // accelerometer data


class Imu
{
public:
    Imu(i2c_inst_t *i2c);
    ~Imu();
    void End();
    void ReadAccelerometer(SensorsData& sensors_data);
    void ReadGyroscope(SensorsData& sensors_data);
    // void ReadTemperature(float& temperature);
    int TestConnection();
    
private:
    void ReadData();
    int AccelerometerDataAvailable();
    int GyroscopeDataAvailable();
    int TemperatureDataAvailable();

    i2c_inst_t *i2c_;
    const int kTemperatureSensitivity{256};
    const int kTemperatureOffset{25};
};

#endif