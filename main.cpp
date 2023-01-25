#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"
#include "lsm303d.h"            // compass
#include "storage.h"            // onboard Flesh memory
#include "pa1010d.h"            // GPS
#include "lsm6dsox/lsm6dsox.h"  // onboard's gyro, accel
// #include "ui/ui.h"              // controls UI
#include "utils/ui.h"           // controls UI
#include "utils/data.h"         // pins, parameters,...


// *************************
// ---Uncomment below line to switch to read mode. 
// #define READ_MEMORY


// ---flags for reading data from sensors
bool read_sensors_flag{false};
// int read_gps_flag{0};
// int gps_fix_count{0};
bool btn1_pressed{false};
bool btn2_pressed{false};



// *************************
bool timer_callback(struct repeating_timer *timer_1)
{
    read_sensors_flag = true;
    return true; 
}

void ButtonCallback(uint gpio, uint32_t events)
{
    printf("ButtonCallback \n");
    if(events == GPIO_IRQ_EDGE_FALL)
    {
        if(gpio == config::kButton_top_pin)
        {
            printf("ButtonCallback BTN 1 \n");
            btn1_pressed = true;
        }
        if(gpio == config::kButton_bottom_pin)
        {
            printf("ButtonCallback BTN 1 \n");
            btn2_pressed = true;
        }
    }
}


int main() {

    stdio_init_all();

    // ---wait for connection to CoolTerm on Mac
    sleep_ms(5000);

    #ifdef ARDUINO_NANO_RP2040
        printf("- ARDUINO 1.0 \n");
    #else
        printf("- Pi Pico \n");
    #endif

    // ---Flash information 
    extern char __flash_binary_end;
    uintptr_t end = (uintptr_t) &__flash_binary_end;
    printf("main: Binary ends at %8x\n", end);
    printf("main: Binary ends at %u\n", end);
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
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(config::kI2C_0_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config::kI2C_0_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config::kI2C_0_sda_pin);
    gpio_pull_up(config::kI2C_0_scl_pin);
    printf("I2C initialized \n");

    // ---UI
    pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(i2c0, config::kOledAddress, pico_ssd1306::Size::W128xH64);
    StateId current_state{StateId::kInit};
    ui::InitButtons();
    // gpio_set_irq_enabled_with_callback(config::kButton_1_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
    // gpio_set_irq_enabled_with_callback(config::kButton_2_pin, GPIO_IRQ_EDGE_FALL, true, ButtonCallback);

    ui::GoToScreen(&display, current_state);

    // ---IMU LSM6DSOX
    float accelerometer[3];
    float gyroscope[3];
    Imu imu{i2c0};
    imu.Begin();

    // --- COMPASS LSM303D
    int magX, magY, magZ;
    lsm303d::init(i2c0);

    // ---GPS PA1010D
    // char numcommand[max_read];
    // std::string latitude{"zero"};
    // std::string longitude{"zero"};
    // std::string utc_time{"zero"};
    char init_command[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    hw_write_masked(&i2c0->hw->sda_hold, 5, I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(config::kI2C_0_sda_pin, config::kI2C_0_scl_pin, GPIO_FUNC_I2C));
    pa1010d::pa1010d_write_command(i2c0, init_command, strlen(init_command));
    
    sleep_ms(5000);

    // ---timer
    struct repeating_timer timer_1;
    add_repeating_timer_ms(100, timer_callback, NULL, &timer_1);

    std::string data_to_store{"|"};

    sleep_ms(10000);



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
        switch (current_state)
        {
        case StateId::kInit:
            // TODO read configuration data from FLASH memory
            sleep_ms(5);
            current_state = StateId::kStandby; 
            ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kStandby:
            if(btn2_pressed)
            {
                extern char __flash_binary_end;
                uintptr_t end = (uintptr_t) &__flash_binary_end;
                printf("while: Binary ends at %8x\n", end);
                printf("while: Binary ends at %u\n", end);

                current_state = StateId::kGpsSearch;
                ui::GoToScreen(&display, StateId::kGpsSearch);
                // turn on GPS, get connection
                btn2_pressed = false;
            }
            break;
        case StateId::kGpsSearch:
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                // turn off GPS
                btn2_pressed = false;
            }
            if(pa1010d::HasFix(i2c0))
            {
                current_state = StateId::kGpsReady;
                ui::GoToScreen(&display, StateId::kGpsReady);
            }
            break;
        case StateId::kGpsReady:
            if(btn1_pressed)
            {
                current_state = StateId::kTraining;
                ui::GoToScreen(&display, StateId::kTraining);
                // turn on SENSORS and set up storing in FLASH
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                // turn off GPS
                btn2_pressed = false;
            }
            // if no action without 30s, turn off GPS and go to kStandby
            break;
        case StateId::kTraining:

            break;
        case StateId::kStopTraining:

            break;
        default:
            break;
        }





// #ifndef READ_MEMORY
//         if(read_sensors_flag)
//         {
//             read_sensors_flag = false;
            
//             if(gps_fix_count < 20)
//             // if(gps_fix_count < 20)  // if GPS does NOT have a fix
//             {
//                 ++read_gps_flag;
//                 if(read_gps_flag == 10)
//                 {
//                     read_gps_flag = 0;

//                     // Clear array
//                     memset(numcommand, 0, max_read);
//                     // Read and re-format
//                     pa1010d::read_raw(I2C_1, numcommand);
//                     pa1010d::parse_GNMRC(numcommand, "GNRMC", latitude, longitude, utc_time);
                    
//                     // printf("---NO fix\n");
//                     // printf("UTC time: %s.\n", utc_time.c_str());
//                     // printf("latitude: %s, longitude: %s.\n", latitude.c_str(), longitude.c_str());
//                     // printf("---\n");

//                     if(latitude != "none")
//                     {
//                         ++gps_fix_count; 
//                     }
//                     else if(gps_fix_count > 0)
//                     {
//                         --gps_fix_count;
//                     }
//                 }
//             }
//             else    // if GPS has a fix
//             {
//                 data_to_store += "|";

//                 // ---GPS
//                 ++read_gps_flag;
//                 if(read_gps_flag == 10)
//                 {
//                     read_gps_flag = 0;
                    
//                     // Clear array
//                     memset(numcommand, 0, max_read);
//                     pa1010d::read_raw(I2C_1, numcommand);
//                     pa1010d::parse_GNMRC(numcommand, "GNRMC", latitude, longitude, utc_time);
//                     data_to_store += utc_time;
//                     data_to_store += ",";
//                     data_to_store += latitude;
//                     data_to_store += ",";
//                     data_to_store += longitude;
//                     data_to_store += ",";
//                 }
//                 // ---LSM303D - magnetic data
//                 lsm303d::read(I2C_1, &magX, &magY, &magZ);
//                 std::string compass_coordinates = std::to_string(magX) + "," + std::to_string(magY) + "," + std::to_string(magZ);
//                 data_to_store += compass_coordinates;
//                 data_to_store += ",";

//                 // ---IMU LSM6DSOX
//                 imu.ReadAccelerometer(accelerometer[0], accelerometer[1], accelerometer[2]);
//                 imu.ReadGyroscope(gyroscope[0], gyroscope[1], gyroscope[2]);
//                 std::string x{"x:" + std::to_string(gyroscope[0])};
//                 std::string y{"y:" + std::to_string(gyroscope[1])};
//                 std::string z{"z:" + std::to_string(gyroscope[2])};
//                 // display.clear();
//                 // drawText(&display, font_12x16, x.c_str(), 0, 0);
//                 // drawText(&display, font_12x16, y.c_str(), 0, 20);
//                 // drawText(&display, font_12x16, z.c_str(), 0, 40);
//                 // display.sendBuffer();


//                 // std::string pages = "| " + std::to_string(saved_pages_counter) + " |";
//                 // display.clear();
//                 // drawText(&display, font_12x16, utc_time.c_str(), 0 ,0);
//                 // drawText(&display, font_12x16, pages.c_str(), 0, 20);
//                 // drawText(&display, font_12x16, latitude.c_str(), 0, 40);
//                 // display.sendBuffer();

//                 if(data_to_store.length() > FLASH_PAGE_SIZE)
//                 {
//                     std::string string_to_store = data_to_store.substr(0, FLASH_PAGE_SIZE);
//                     data_to_store.erase(0, FLASH_PAGE_SIZE);
//                     printf("%s", string_to_store.c_str());
//                     bool success = storage::save_string_in_flash(string_to_store);
//                     if(!success)
//                     {
//                         printf("ERROR: flash memory full\n");
//                         // display.clear();
//                         // drawText(&display, font_12x16, "ERROR:", 0 ,0);
//                         // drawText(&display, font_12x16, "flash memory", 0, 20);
//                         // drawText(&display, font_16x32, "full", 0, 40);
//                         // display.sendBuffer();

//                         sleep_ms(10000);

//                         return 0;
//                     }
//                 }
//             }
//         }

// #endif
    }
}