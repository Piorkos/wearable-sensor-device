#ifndef LSM303D_H
#define LSM303D_H

#include "hardware/i2c.h"
#include "../utils/sensors_data.h"


#define LSM303D_I2C_ADDR    0x1d
#define LSM303D_WHOAMI      73
//--- Control register addresses -- from LSM303D datasheet
#define CTRL_0 31 //--- General settings
#define CTRL_1 32 //--- Turns on accelerometer and configures data rate
#define CTRL_2 33 //--- Self test accelerometer, anti-aliasing accel filter
#define CTRL_3 34 //--- Interrupts
#define CTRL_4 35 //--- Interrupts
#define CTRL_5 36 //--- Turns on temperature sensor
#define CTRL_6 37 //--- Magnetic resolution selection, data rate config
#define CTRL_7 38 //--- Turns on magnetometer and adjusts mode


class Compass
{
public:
    Compass();
    ~Compass();
    void Read(SensorsData& sensors_data);
    int TestConnection();

private:
    int CombineTwoRegisters(int MSB, int LSB);

    //--- Registers holding twos-complemented MSB and LSB of magnetometer readings -- from LSM303D datasheet
    static constexpr uint8_t MAG_X_LSB{8}; //---  x
    static constexpr uint8_t MAG_X_MSB{9};
    static constexpr uint8_t MAG_Y_LSB{10}; //---  y
    static constexpr uint8_t MAG_Y_MSB{11};
    static constexpr uint8_t MAG_Z_LSB{12}; //---  z
    static constexpr uint8_t MAG_Z_MSB{13};

    i2c_inst_t *i2c_{i2c1};
};

#endif //LSM303D_H