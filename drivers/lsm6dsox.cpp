#include "lsm6dsox.h"


Imu::Imu(i2c_inst_t *i2c)
:i2c_{i2c}
{
    printf("Imu Constructor \n");


    uint8_t buffer[1];
    uint8_t reg{LSM6DSOX_WHO_AM_I};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true); // true to keep master control of bus
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, buffer, 1, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU ADDRESS READING ERROR, bytes= %d \n", bytesRead);
    }
    if (!(buffer[0] == 0x6C || buffer[0] == 0x69))
    {
        printf("WRONG IMU ADDRESS \n");
    }
    else
    {
        printf("CORRECT IMU ADDRESS \n");
    }

  
    //set the gyroscope control register to work at 104 Hz, 2000 dps and in bypass mode
    uint8_t buf[] = {LSM6DSOX_CTRL2_G, 0x4C};
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);

    // Set the Accelerometer control register to work at 104 Hz, 4 g,and in bypass mode and enable ODR/4
    // low pass filter (check figure9 of LSM6DSOX's datasheet)
    buf[0] = LSM6DSOX_CTRL1_XL;
    buf[1] = 0x4E;
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);

    // set gyroscope power mode to high performance and bandwidth to 16 MHz
    buf[0] = LSM6DSOX_CTRL7_G;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);

    // Set the ODR config register to ODR/4
    buf[0] = LSM6DSOX_CTRL8_XL;
    buf[1] = 0x09;
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);
}

Imu::~Imu()
{
    // printf("Imu Destructor \n");
}

/**
 * TODO: at the moment this is not used and I don't remember why. Try to understand it, because probably it should be used.
 * @brief 
 * 
 */
void Imu::End()
{
    uint8_t buf[] = {LSM6DSOX_CTRL2_G, 0x00};
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);

    buf[0] = LSM6DSOX_CTRL1_XL;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, buf, 2, false);
}

// void Imu::ReadAccelerometer(float& x, float& y, float& z)
void Imu::ReadAccelerometer(SensorsData& sensors_data)
{
    printf("Imu::ReadAccelerometer \n");
    
    int16_t buf[3];
    uint8_t reg{LSM6DSOX_OUTX_L_A};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 6, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU ACCEL READING ERROR, bytes= %d \n", bytesRead);
    }
    else
    {
        sensors_data.accelerometer[0] = buf[0] * 8.0 / 32768.0;
        sensors_data.accelerometer[1] = buf[1] * 8.0 / 32768.0;
        sensors_data.accelerometer[2] = buf[2] * 8.0 / 32768.0;
    }

    printf("Accelerometer: x=%f, y=%f, z=%f \n", sensors_data.accelerometer[0], sensors_data.accelerometer[1], sensors_data.accelerometer[2]);
}

int Imu::AccelerometerDataAvailable()
{
    uint8_t buf[1];
    uint8_t reg{LSM6DSOX_STATUS_REG};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 1, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU ACCEL READING ERROR, bytes= %d \n", bytesRead);
    }
    else if(buf[0] & 0x01)
    {
        return 1;
    }
    return 0;
}

void Imu::ReadGyroscope(SensorsData& sensors_data)
{
    printf("Imu::ReadGyroscope \n");
    int16_t buf[3];
    uint8_t reg{LSM6DSOX_OUTX_L_G};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 6, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU GYROSCOPE READING ERROR, bytes= %d \n", bytesRead);
    }
    else
    {
        sensors_data.gyroscope[0] = buf[0] * 2000.0 / 32768.0;
        sensors_data.gyroscope[1] = buf[1] * 2000.0 / 32768.0;
        sensors_data.gyroscope[2] = buf[2] * 2000.0 / 32768.0;
    }

    printf("Gyroscop: x=%f, y=%f, z=%f \n", sensors_data.gyroscope[0], sensors_data.gyroscope[1], sensors_data.gyroscope[2]);
}

int Imu::GyroscopeDataAvailable()
{
    uint8_t buf[1];
    uint8_t reg{LSM6DSOX_STATUS_REG};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 1, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU GYROSCOPE READING ERROR, bytes= %d \n", bytesRead);
    }
    else if(buf[0] & 0x02)
    {
        return 1;
    }
    return 0;
}

// There is something wrong with the read value, the temperature is to high.
// void Imu::ReadTemperature(float& temperature)
// {
//     int16_t buf[1];
//     uint8_t reg{LSM6DSOX_OUT_TEMP_L};

//     i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
//     int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 1, false);
//     if(bytesRead == PICO_ERROR_GENERIC)
//     {
//         // printf("IMU TEMPERATURE READING ERROR, bytes= %d \n", bytesRead);
//     }
//     else
//     {
//         temperature = (static_cast<float>(buf[0]) / kTemperatureSensitivity) + kTemperatureOffset;
//     }
// }

int Imu::TemperatureDataAvailable()
{
    uint8_t buf[1];
    uint8_t reg{LSM6DSOX_STATUS_REG};

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &reg, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, (uint8_t*)buf, 1, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU TEMPERATURE READING ERROR, bytes= %d \n", bytesRead);
    }
    else if(buf[0] & 0x04)
    {
        return 1;
    }
    return 0;
}

void Imu::ReadData()
{
    uint8_t buffer[6];
    uint8_t val = 0x3B;

    i2c_write_blocking(i2c_, LSM6DSOX_ADDRESS, &val, 1, true);
    int bytesRead = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, buffer, 6, false);
    if(bytesRead == PICO_ERROR_GENERIC)
    {
        printf("IMU READING ERROR, bytes= %d \n", bytesRead);
    }
}

/**
 * @brief Checks if connection is ok.
 * 
 * @return -2 if timeout, -1 if connection broken, 1 if connection ok
 */
int Imu::TestConnection()
{
    uint8_t buffer;
    int bytes_read = i2c_read_blocking(i2c_, LSM6DSOX_ADDRESS, &buffer, 1, false);
    if(bytes_read == PICO_ERROR_GENERIC)
    {
        printf("IMU READING ERROR - NO CONNECTION \n");
        return -2;
    }
    else if(bytes_read == PICO_ERROR_TIMEOUT)
    {
        printf("IMU READING ERROR - TIMEOUT \n");
        return -1;
    }
    return 1;
}