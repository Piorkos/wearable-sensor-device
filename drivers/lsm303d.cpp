#include "lsm303d.h"

Compass::Compass()
{
    uint8_t buf[] = {CTRL_1, 0x57};
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, buf, 2, false);
    buf[0] = CTRL_2;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, buf, 2, false);
    buf[0] = CTRL_5;
    buf[1] = 0x64;
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, buf, 2, false);
    buf[0] = CTRL_6;
    buf[1] = 0x20;
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, buf, 2, false);
    buf[0] = CTRL_7;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, buf, 2, false);
}

Compass::~Compass()
{
    i2c_ = nullptr;
    delete i2c_;
}

/**
 * @brief Reads data from the magnetometer and stores it in sensors_data.
 * 
 * @param sensors_data Reference to a variable in which the data needs to be stored.
 */
void Compass::Read(SensorsData& sensors_data)
{
    uint8_t bufferM[1];
    uint8_t bufferL[1];
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_X_MSB, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferM, 1, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("COMPAS READING ERROR, bytes= %d \n", bytesRead);
    }
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_X_LSB, 1, true);
    i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferL, 1, false);
    sensors_data.mag_x = CombineTwoRegisters(bufferM[0], bufferL[0]);
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_Y_MSB, 1, true);
    i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferM, 1, false);
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_Y_LSB, 1, true);
    i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferL, 1, false);
    sensors_data.mag_y = CombineTwoRegisters(bufferM[0], bufferL[0]);
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_Z_MSB, 1, true);
    i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferM, 1, false);
    i2c_write_blocking(i2c_, LSM303D_I2C_ADDR, &MAG_Z_LSB, 1, true);
    i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, bufferL, 1, false);
    sensors_data.mag_z = CombineTwoRegisters(bufferM[0], bufferL[0]);
}

/**
 * @brief Checks if connection is ok.
 * 
 * @return -2 if timeout, -1 if connection broken, 1 if connection ok
 */
int Compass::TestConnection()
{
    uint8_t buffer;
    int bytes_read = i2c_read_blocking(i2c_, LSM303D_I2C_ADDR, &buffer, 1, false);
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

int Compass::CombineTwoRegisters(int MSB, int LSB)
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