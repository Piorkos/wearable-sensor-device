#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/binary_info.h"
#include <string>


#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"
#include "lsm303d.h"    // compass
#include "pa1010d.h"    // GPS
#include "mpu6050.h"    // gyro, accel
#include "storage.h"    // Pico onboard Flash memory


// *************************
// ---Uncomment below line to switch to read mode. 
// #define READ_MEMORY


// ---I2C
#define I2C_1 i2c1
#define I2C_1_SDA_PIN 2
#define I2C_1_SCL_PIN 3
#define I2C_2 i2c0
#define I2C_2_SDA_PIN 4
#define I2C_2_SCL_PIN 5

// ---addresses
#define OLED_ADDR _u(0x3C)

// ---flags for reading data from sensors
bool read_sensors_flag{false};
int read_gps_flag{0};
int gps_fix_count{0};


// *************************
bool timer_callback(struct repeating_timer *timer_1)
{
    read_sensors_flag = true;

    // printf("TIMER\n");

    return true;
}


int main() {

    stdio_init_all();

    // ---wait for connection to CoolTerm on Mac
    sleep_ms(15000);
    // ---
    printf("IO initialized\n");

    extern char __flash_binary_end;
    uintptr_t end = (uintptr_t) &__flash_binary_end;
    printf("Binary ends at %8x\n", end);
    printf("Binary ends at %u\n", end);
    printf("flash_target_contents %8x\n", flash_target_contents);
    printf("flash_target_contents %u\n", flash_target_contents);
    printf("max_pages %u\n", max_pages);
    printf("XIP_BASE %8x\n", XIP_BASE);
    printf("XIP_BASE %u\n", XIP_BASE);
    printf("FLASH_TARGET_OFFSET %8x\n", FLASH_TARGET_OFFSET);
    printf("FLASH_TARGET_OFFSET %u\n", FLASH_TARGET_OFFSET);
    printf("FLASH_PAGE_SIZE %8x\n", FLASH_PAGE_SIZE);
    printf("FLASH_PAGE_SIZE %u\n", FLASH_PAGE_SIZE);
    printf("PICO_FLASH_SIZE_BYTES %8x\n", PICO_FLASH_SIZE_BYTES);
    printf("PICO_FLASH_SIZE_BYTES %u\n", PICO_FLASH_SIZE_BYTES);
    printf("FLASH_TARGET_OFFSET / FLASH_PAGE_SIZE %u\n", (FLASH_TARGET_OFFSET/FLASH_PAGE_SIZE));
    printf("PICO_FLASH_SIZE_BYTES / FLASH_PAGE_SIZE %u\n", (PICO_FLASH_SIZE_BYTES/FLASH_PAGE_SIZE));
    printf("(PICO_FLASH_SIZE_BYTES - start) / FLASH_PAGE_SIZE %u\n", ((PICO_FLASH_SIZE_BYTES - (XIP_BASE + FLASH_TARGET_OFFSET))/FLASH_PAGE_SIZE));


    // *************************
    // ------INITIALIZATION
    // ---I2C
    i2c_init(I2C_2, 400 * 1000);
    gpio_set_function(I2C_2_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_2_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_2_SDA_PIN);
    gpio_pull_up(I2C_2_SCL_PIN);
    i2c_init(I2C_1, 400 * 1000);
    gpio_set_function(I2C_1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_1_SDA_PIN);
    gpio_pull_up(I2C_1_SCL_PIN);
    printf("I2C initialized \n");

    // ---OLED
    //Create a new display object
    pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(I2C_2, 0x3C, pico_ssd1306::Size::W128xH64);
    printf("display initialized \n");
    //For font 8x8, max letters in line - 16
    //Max number of lines 7,5 with 2px of space between lines.
    drawText(&display, font_8x8, "display init", 0 ,0);
    display.sendBuffer(); //Send buffer to device and show on screen
    printf("display tested \n");
    sleep_ms(1000);

    // ---LSM303D
    int magX, magY, magZ;
    lsm303d::init(I2C_1);
    printf("compass initialized \n");
    display.clear();
    drawText(&display, font_8x8, "display init", 0 ,0);
    drawText(&display, font_8x8, "compass init", 0, 10);
    display.sendBuffer(); //Send buffer to device and show on screen
    sleep_ms(1000);

    // ---PA1010D
    char numcommand[max_read];
    std::string latitude{"zero"};
    std::string longitude{"zero"};
    std::string utc_time{"zero"};
    // Decide which protocols you would like to retrieve data from
    char init_command[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    hw_write_masked(&I2C_1->hw->sda_hold, 5, I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(I2C_1_SDA_PIN, I2C_1_SCL_PIN, GPIO_FUNC_I2C));
    pa1010d::pa1010d_write_command(I2C_1, init_command, strlen(init_command));
    printf("GPS initialized \n");
    display.clear();
    drawText(&display, font_8x8, "display init", 0 ,0);
    drawText(&display, font_8x8, "compass init", 0, 10);
    drawText(&display, font_8x8, "GPS init", 0, 20);
    display.sendBuffer(); //Send buffer to device and show on screen
    sleep_ms(1000);

    // ---MPU6050
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(I2C_2_SDA_PIN, I2C_2_SCL_PIN, GPIO_FUNC_I2C));
    mpu6050::mpu6050_reset(I2C_2);
    int16_t acceleration[3], gyro[3], temp;
    display.clear();
    drawText(&display, font_8x8, "display init", 0 ,0);
    drawText(&display, font_8x8, "compass init", 0, 10);
    drawText(&display, font_8x8, "GPS init", 0, 20);
    drawText(&display, font_8x8, "gyro init", 0, 30);
    display.sendBuffer(); //Send buffer to device and show on screen
    sleep_ms(2000);

    // ---timer
    struct repeating_timer timer_1;
    add_repeating_timer_ms(100, timer_callback, NULL, &timer_1);

    std::string data_to_store{"|"};


#ifdef READ_MEMORY
    printf("---Reading data: \n");

    display.clear();
    drawText(&display, font_8x8, "READING DATA", 0 ,0);
    display.sendBuffer(); //Send buffer to device and show on screen
    sleep_ms(10000);

    saved_pages_counter = max_pages;
    // saved_pages_counter = 6000;
    printf("saved_pages_counter %u\n", saved_pages_counter);

    for (size_t i = 0; i < saved_pages_counter; ++i)
    {
        storage::read_data_from_flash(i);
        // if(i % 300 == 0)
        // {
        //     sleep_ms(10000);
        // }
        // if(i % 50 == 0)
        // {
        //     sleep_ms(500);
        // }
    }
    printf("---Reading data finished\n");
#endif



    while(true)
    {

#ifndef READ_MEMORY

        tight_loop_contents();

        if(read_sensors_flag)
        {
            read_sensors_flag = false;
            
            if(gps_fix_count < 20)  // if GPS does NOT have a fix
            {
                ++read_gps_flag;
                if(read_gps_flag == 10)
                {
                    read_gps_flag = 0;

                    // Clear array
                    memset(numcommand, 0, max_read);
                    // Read and re-format
                    pa1010d::read_raw(I2C_1, numcommand);
                    pa1010d::parse_GNMRC(numcommand, "GNRMC", latitude, longitude, utc_time);
                    
                    printf("---NO fix\n");
                    printf("UTC time: %s.\n", utc_time.c_str());
                    printf("latitude: %s, longitude: %s.\n", latitude.c_str(), longitude.c_str());
                    printf("---\n");

                    display.clear();
                    drawText(&display, font_8x8, "   GPS NO FIX", 0 ,0);
                    drawText(&display, font_8x8, "time:", 0, 12);
                    drawText(&display, font_8x8, utc_time.c_str(), 0, 22);
                    drawText(&display, font_8x8, "coordinates:", 0, 32);
                    drawText(&display, font_8x8, latitude.c_str(), 0, 42);
                    drawText(&display, font_8x8, longitude.c_str(), 0, 52);
                    display.sendBuffer();

                    if(latitude != "none")
                    {
                        ++gps_fix_count; 
                    }
                    else if(gps_fix_count > 0)
                    {
                        --gps_fix_count;
                    }
                }
            }
            else    // if GPS has a fix
            {
                data_to_store += "|";

                // ---GPS
                ++read_gps_flag;
                if(read_gps_flag == 10)
                {
                    read_gps_flag = 0;
                    
                    // Clear array
                    memset(numcommand, 0, max_read);
                    pa1010d::read_raw(I2C_1, numcommand);
                    pa1010d::parse_GNMRC(numcommand, "GNRMC", latitude, longitude, utc_time);
                    data_to_store += utc_time;
                    data_to_store += ",";
                    data_to_store += latitude;
                    data_to_store += ",";
                    data_to_store += longitude;
                    data_to_store += ",";
                }
// ---LSM303D - magnetic data
                lsm303d::read(I2C_1, &magX, &magY, &magZ);
                std::string compass_coordinates = std::to_string(magX) + "," + std::to_string(magY) + "," + std::to_string(magZ);
                data_to_store += compass_coordinates;
                data_to_store += ",";


                // ---MPU6050 - accelerometer & gyroscope
                mpu6050::mpu6050_read_raw(I2C_2, acceleration, gyro, &temp);
                std::string accel_str = std::to_string(acceleration[0]/2048.0) + "," + std::to_string(acceleration[1]/2048.0) + "," + std::to_string(acceleration[2]/2048.0);
                std::string gyr_str = std::to_string(gyro[0]/16.4) + "," + std::to_string(gyro[1]/16.4) + "," + std::to_string(gyro[2]/16.4);
                data_to_store += accel_str;
                data_to_store += ",";
                data_to_store += gyr_str;


                std::string pages = "| " + std::to_string(saved_pages_counter) + " |";
                display.clear();
                drawText(&display, font_12x16, utc_time.c_str(), 0 ,0);
                drawText(&display, font_12x16, pages.c_str(), 0, 20);
                drawText(&display, font_12x16, latitude.c_str(), 0, 40);
                display.sendBuffer();

                if(data_to_store.length() > FLASH_PAGE_SIZE)
                {
                    std::string string_to_store = data_to_store.substr(0, FLASH_PAGE_SIZE);
                    data_to_store.erase(0, FLASH_PAGE_SIZE);
                    printf("%s", string_to_store.c_str());
                    bool success = storage::save_string_in_flash(string_to_store);
                    if(!success)
                    {
                        printf("ERROR: flash memory full\n");
                        display.clear();
                        drawText(&display, font_12x16, "ERROR:", 0 ,0);
                        drawText(&display, font_12x16, "flash memory", 0, 20);
                        drawText(&display, font_16x32, "full", 0, 40);
                        display.sendBuffer();

                        sleep_ms(10000);

                        return 0;
                    }
                }
            }
        }

#endif

    }
}
