#ifndef LSM303D_H
#define LSM303D_H

#include "hardware/i2c.h"
#include "../utils/sensors_data.h"


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
static constexpr uint8_t MAG_X_LSB{8}; //---  x
static constexpr uint8_t MAG_X_MSB{9};
static constexpr uint8_t MAG_Y_LSB{10}; //---  y
static constexpr uint8_t MAG_Y_MSB{11};
static constexpr uint8_t MAG_Z_LSB{12}; //---  z
static constexpr uint8_t MAG_Z_MSB{13};


int combineTwoRegister(int MSB, int LSB)
{
	//std::cout<<"MSB: " << MSB << " LSB: " << LSB<<std::endl;
	int combined = 256 * MSB + LSB;
	if(combined >= 32768)
    {
		return combined - 65536;
	}
    else
    {
		return combined;
	}
}

namespace lsm303d
{
    void init(i2c_inst_t *i2c)
    {
        uint8_t buf[] = {CTRL_1, 0x57};
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, buf, 2, false);
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

    void read(i2c_inst_t *i2c, SensorsData& sensors_data)
    // void read(i2c_inst_t *i2c, int *magX, int *magY, int *magZ)
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
        sensors_data.mag_x = combineTwoRegister(bufferM[0], bufferL[0]);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Y_MSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferM, 1, false);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Y_LSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferL, 1, false);
        sensors_data.mag_y = combineTwoRegister(bufferM[0], bufferL[0]);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Z_MSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferM, 1, false);
        i2c_write_blocking(i2c, LSM303D_I2C_ADDR, &MAG_Z_LSB, 1, true);
        i2c_read_blocking(i2c, LSM303D_I2C_ADDR, bufferL, 1, false);
        sensors_data.mag_z = combineTwoRegister(bufferM[0], bufferL[0]);
    }

    /**
     * @brief Checks if connection is ok.
     * 
     * @param i2c Either i2c0 or i2c1
     * @return -2 if timeout, -1 if connection broken, 1 if connection ok
     */
    int TestConnection(i2c_inst_t *i2c)
    {
        uint8_t buffer;
        int bytes_read = i2c_read_blocking(i2c, LSM303D_I2C_ADDR, &buffer, 1, false);
        if(bytes_read == PICO_ERROR_GENERIC)
        {
            printf("COMPAS READING ERROR - NO CONNECTION \n");
            return -2;
        }
        else if(bytes_read == PICO_ERROR_TIMEOUT)
        {
            printf("COMPAS READING ERROR - TIMEOUT \n");
            return -1;
        }
        return 1;
    }
}


#endif //LSM303D_H