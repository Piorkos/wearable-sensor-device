#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"


static int mpu6050_addr = 0x68;

namespace mpu6050
{
    static void mpu6050_reset(i2c_inst_t *i2c) 
    {
        // Two byte reset. First byte register, second byte data
        // There are a load more options to set up the device in different ways that could be added here
        uint8_t buf[] = {0x6B, 0x00};
        i2c_write_blocking(i2c, mpu6050_addr, buf, 2, false);

        // Change gyroscope range to +-20000 deg/s
        buf[0] = 0x1B;
        // buf[1] = 0b00011000;
        buf[1] = 0x18;
        i2c_write_blocking(i2c, mpu6050_addr, buf, 2, false);

        // Change accelerometer range to +-16 g
        buf[0] = 0x1C;
        // buf[1] = 0b00011000;
        buf[1] = 0x18;
        i2c_write_blocking(i2c, mpu6050_addr, buf, 2, false);
    }

    static void mpu6050_read_raw(i2c_inst_t *i2c, int16_t accel[3], int16_t gyro[3], int16_t *temp)
    {
        // For this particular device, we send the device the register we want to read
        // first, then subsequently read from the device. The register is auto incrementing
        // so we don't need to keep sending the register we want, just the first.

        uint8_t buffer[6];

        // Start reading acceleration registers from register 0x3B for 6 bytes
        uint8_t val = 0x3B;
        i2c_write_blocking(i2c, mpu6050_addr, &val, 1, true); // true to keep master control of bus
        int bytesRead = i2c_read_blocking(i2c, mpu6050_addr, buffer, 6, false);
        if(bytesRead == PICO_ERROR_GENERIC)
        {
            printf("ACCEL READING ERROR, bytes= %d \n", bytesRead);
        }

        for (int i = 0; i < 3; i++)
        {
            accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        // Now gyro data from reg 0x43 for 6 bytes
        // The register is auto incrementing on each read
        val = 0x43;
        i2c_write_blocking(i2c, mpu6050_addr, &val, 1, true);
        bytesRead = i2c_read_blocking(i2c, mpu6050_addr, buffer, 6, false);  // False - finished with bus
        if(bytesRead == PICO_ERROR_GENERIC)
        {
            printf("ACCEL READING ERROR, bytes= %d \n", bytesRead);
        }
        for (int i = 0; i < 3; i++)
        {
            gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        // Now temperature from reg 0x41 for 2 bytes
        // The register is auto incrementing on each read
        val = 0x41;
        i2c_write_blocking(i2c, mpu6050_addr, &val, 1, true);
        bytesRead = i2c_read_blocking(i2c, mpu6050_addr, buffer, 2, false);  // False - finished with bus
        if(bytesRead == PICO_ERROR_GENERIC)
        {
            printf("TEMP READING ERROR, bytes= %d \n", bytesRead);
        }

        *temp = buffer[0] << 8 | buffer[1];
    }
}


#endif //MPU6050_H