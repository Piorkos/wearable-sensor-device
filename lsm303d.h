#ifndef LSM303D_H
#define LSM303D_H

#include "hardware/i2c.h"


#define LSM303D_I2C_ADDR 0x1d
#define LSM303D_WHOAMI 73
//--- Control register addresses -- from LSM303D datasheet
#define CTRL_0 31 //--- General settings
#define CTRL_1 32 //--- Turns on accelerometer and configures data rate
#define CTRL_2 33 //--- Self test accelerometer, anti-aliasing accel filter
#define CTRL_3 34 //--- Interrupts
#define CTRL_4 35 //--- Interrupts
#define CTRL_5 36 //--- Turns on temperature sensor
#define CTRL_6 37 //--- Magnetic resolution selection, data rate config
#define CTRL_7 38 //--- Turns on magnetometer and adjusts mode
//--- Registers holding twos-complemented MSB and LSB of magnetometer readings -- from LSM303D datasheet
const uint8_t MAG_X_LSB{8}; //---  x
const uint8_t MAG_X_MSB{9};
const uint8_t MAG_Y_LSB{10}; //---  y
const uint8_t MAG_Y_MSB{11};
const uint8_t MAG_Z_LSB{12}; //---  z
const uint8_t MAG_Z_MSB{13};


int combineTwoRegister(int MSB, int LSB){
	//std::cout<<"MSB: " << MSB << " LSB: " << LSB<<std::endl;
	int combined = 256 * MSB + LSB;
	if(combined >= 32768){
		return combined - 65536;
	} else {
		return combined;
	}
}

namespace lsm303d
{
    void init(i2c_inst_t *i2c)
    {
        uint8_t buf[] = {CTRL_1, 0x57};
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, true);
        buf[0] = CTRL_2;
        buf[1] = 0x00;
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, false);
        buf[0] = CTRL_5;
        buf[1] = 0x64;
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, false);
        buf[0] = CTRL_6;
        buf[1] = 0x20;
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, false);
        buf[0] = CTRL_7;
        buf[1] = 0x00;
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, false);
    }

    void read(i2c_inst_t *i2c, int *magX, int *magY, int *magZ)
    {
        uint8_t bufferM[1];
        uint8_t bufferL[1];
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_X_MSB, 1, true);
        int bytesRead = i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferM, 1, false);
        if(bytesRead == PICO_ERROR_GENERIC)
        {
            printf("COMPAS READING ERROR, bytes= %d \n", bytesRead);
        }
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_X_LSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferL, 1, false);
        *magX = combineTwoRegister(bufferM[0], bufferL[0]);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Y_MSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferM, 1, false);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Y_LSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferL, 1, false);
        *magY = combineTwoRegister(bufferM[0], bufferL[0]);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Z_MSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferM, 1, false);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Z_LSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferL, 1, false);
        *magZ = combineTwoRegister(bufferM[0], bufferL[0]);
    }
}


#endif //LSM303D_H