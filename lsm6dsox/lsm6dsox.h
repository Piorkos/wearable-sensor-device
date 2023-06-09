#ifndef LSM6DSOX_H
#define LSM6DSOX_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../utils/sensors_data.h"

class Imu
{
private:
    i2c_inst_t *i2c_;
    const int kTemperatureSensitivity{256};
    const int kTemperatureOffset{25};
public:
    Imu(i2c_inst_t *i2c);
    void Begin();
    void End();
    void ReadAccelerometer(SensorsData& sensors_data);
    void ReadGyroscope(SensorsData& sensors_data);
    // void ReadTemperature(float& temperature);
    int AccelerometerDataAvailable();
    int GyroscopeDataAvailable();
    int TemperatureDataAvailable();
    int TestConnection();
    
private:
    void ReadData();
};

#endif