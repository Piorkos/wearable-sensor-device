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
#include "drivers/lsm303d.h"            // compass
#include "drivers/storage.h"            // onboard Flesh memory
#include "drivers/pa1010d.h"            // GPS
#include "lsm6dsox/lsm6dsox.h"  // onboard's gyro, accel
#include "utils/ui.h"           // controls UI
#include "utils/data.h"         // pins, parameters,...
#include "utils/sensors_data.h" // Struct to hold data from sensors
#include "utils/distance.h"


// *************************
// ---Uncomment below line to switch to TEST mode. 
// #define I2C_CONNECTION_TEST


// ---flags for reading data from sensors
bool read_sensors_flag{false};
bool btn1_pressed{false};   // top
bool btn2_pressed{false};   // bottom
uint16_t no_activity_counter{0};


// *************************
bool timer_callback(struct repeating_timer *timer_1)
{
    read_sensors_flag = true;
    // ++no_activity_counter;
    return true;
}

void ButtonCallback(uint gpio, uint32_t events)
{
    printf("ButtonCallback \n");
    if(events == GPIO_IRQ_EDGE_FALL)
    {
        if(gpio == config::kButton_left_pin)
        {
            printf("ButtonCallback BTN 1 \n");
            btn1_pressed = true;
        }
        if(gpio == config::kButton_right_pin)
        {
            printf("ButtonCallback BTN 2 \n");
            btn2_pressed = true;
        }
    }
}


int main() {

    stdio_init_all();

    // ---wait for connection to CoolTerm on Mac
    sleep_ms(3000);

    #ifdef ARDUINO_NANO_RP2040
        printf("- ARDUINO 1.2 \n");
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
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(config::kI2C_1_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config::kI2C_1_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config::kI2C_1_sda_pin);
    gpio_pull_up(config::kI2C_1_scl_pin);
    printf("I2C initialized \n");

    // ---UI
    pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(i2c1, config::kOledAddress, pico_ssd1306::Size::W128xH64);
    StateId current_state{StateId::kInit};
    ui::InitButtons();
    printf("UI initialized \n");

    ui::GoToScreen(&display, current_state);

    // ---IMU LSM6DSOX
    Imu imu{i2c0};
    imu.Begin();
    printf("IMU initialized \n");

    // --- COMPASS LSM303D
    std::string compass_coordinates{};
    lsm303d::init(i2c1);
    printf("COMPASS initialized \n");

    // ---GPS PA1010D
    printf("MAX READ = %i \n", max_read);
    char init_command[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    hw_write_masked(&i2c1->hw->sda_hold, 5, I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(config::kI2C_0_sda_pin, config::kI2C_0_scl_pin, GPIO_FUNC_I2C));
    pa1010d::init(i2c1, init_command, strlen(init_command));

    // ---timer
    struct repeating_timer timer_1;
    add_repeating_timer_ms(100, timer_callback, NULL, &timer_1);


    SensorsData sensors_data{};
    
    std::string data_to_store{"|"};

    // std::string string_to_save{"AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD"};
    std::string string_to_save{"$1|-470;-658;526;-0.107666;-0.242432;0.965332;2.014160;-1.708984;-1.098633|-470;-658;526;-0.105103;-0.230225;0.971924;-6.225586;3.051758;-0.427246|-465;-625;519;-0.116943;-0.250000;0.947754;-0.549316;-0.183105;-0.610352|-465;-625;519;-0.119507;-0.256226;0."};
    int debug_counter{0};
    sensors_data.accelerometer[0] = 11.0;
    sensors_data.accelerometer[1] = 12.0;
    sensors_data.accelerometer[2] = 13.0;
    sensors_data.gyroscope[0] = 21.0;
    sensors_data.gyroscope[1] = 22.0;
    sensors_data.gyroscope[2] = 23.0;
    sensors_data.mag_x = 1;
    sensors_data.mag_y = 2;
    sensors_data.mag_z = 3;
    storage::EraseData();

    while(true)
    {
        // printf("while current state = %i \n", current_state);
        
#ifdef I2C_CONNECTION_TEST
        
        int response{0};
        std::string msg_0{"COMP-"};
        std::string msg_1{"GPS-"};
        std::string msg_2{"IMU-"};

        response = lsm303d::TestConnection(i2c1);
        if(response == -1){
            msg_0.append("timeout");
        }else if(response == -2){
            msg_0.append("conn");
        }
        response = pa1010d::TestConnection(i2c1);
        if(response == -1){
            msg_1.append("timeout");
        }else if(response == -2){
            msg_1.append("conn");
        }
        response = imu.TestConnection();
        if(response == -1){
            msg_2.append("timeout");
        }else if(response == -2){
            msg_2.append("conn");
        }
        printf("MAIN 0: %s \n", msg_0.c_str());
        printf("MAIN 1: %s \n", msg_1.c_str());
        printf("MAIN 2: %s \n", msg_2.c_str());
        ui::GoToScreen(&display, StateId::kError, msg_0, msg_1, msg_2);
        sleep_ms(2);

#else

        switch (current_state)
        {
        case StateId::kInit:            
            storage::RestoreSavedPagesCounter();
            sleep_ms(3000);
            current_state = StateId::kStandby;
            ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kStandby:
            if(btn1_pressed)
            {
                current_state = StateId::kReadData;
                ui::GoToScreen(&display, StateId::kReadData); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kGpsSearch;
                ui::GoToScreen(&display, StateId::kGpsSearch);
                // TODO turn on GPS, get connection
                btn2_pressed = false;
            }
            break;
        case StateId::kGpsSearch:
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn2_pressed = false;
            }
            if(read_sensors_flag)
            {
                read_sensors_flag = false;
                int has_fix{false};
                has_fix = pa1010d::HasFix(i2c1, sensors_data);
                if(has_fix)
                {
                    no_activity_counter = 0;
                    current_state = StateId::kGpsReady;
                    ui::GoToScreen(&display, StateId::kGpsReady);
                }
            }
            break;
        case StateId::kGpsReady:
            if(btn1_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                storage::StartNewTraining();
                current_state = StateId::kTraining;
                ui::GoToScreen(&display, StateId::kTraining);
                // TODO turn on SENSORS and set up storing in FLASH
                btn2_pressed = false;
            }
            if(no_activity_counter > 300)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
            }
            break;
        case StateId::kTraining:
            // printf("StateId::kTraining \n");
            if(read_sensors_flag)
            {
                printf("StateId::kTraining read_sensors_flag \n");
                read_sensors_flag = false;
                std::string error_msg{""};

                // -- compass - LSM303D
                lsm303d::read(i2c1, sensors_data);
                // -- gyro, accel - LSM6DSOX
                imu.ReadAccelerometer(sensors_data);
                imu.ReadGyroscope(sensors_data);
                // -- gps & disntace
                int gps_error{0};
                bool gps_data = pa1010d::ReadData1Per10(i2c1, sensors_data, gps_error);
                if (gps_error < 0)
                {
                    error_msg.append(":GPS");
                    pa1010d::init(i2c1, init_command, strlen(init_command));
                }
                sensors_data.distance = 0;
                if(gps_data)
                {
                    distance::CalculateDistance(sensors_data);
                }
                // ---write data to FLASH
                int success = storage::UpdateDataToStore(sensors_data, gps_data);
                printf("MAIN::training: UpdateDataToStore result = %i \n", success);

                ui::UpdateTraining(&display, sensors_data.distance, error_msg);
            }
            
            if(btn2_pressed)
            {
                no_activity_counter = 0;
                current_state = StateId::kStopTraining;
                ui::GoToScreen(&display, StateId::kStopTraining);
                btn2_pressed = false;
            }
            break;
        case StateId::kStopTraining:
            if(btn1_pressed)
            {
                current_state = StateId::kTraining;
                ui::GoToScreen(&display, StateId::kTraining);
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn2_pressed = false;
            }
            if(no_activity_counter > 300)
            {
                current_state = StateId::kTraining;
                ui::GoToScreen(&display, StateId::kTraining);
            }
            break;
        case StateId::kReadData:
            if(btn1_pressed)
            {
                current_state = StateId::kEraseData;
                ui::GoToScreen(&display, StateId::kEraseData); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kReadingInProgress;
                ui::GoToScreen(&display, StateId::kReadingInProgress);
                btn2_pressed = false;
            }
            
            break;
        case StateId::kReadingInProgress:
            // Starts reading data and sending via UART to PC
            storage::ReadAllData();
            sleep_ms(3);

            current_state = StateId::kStandby;
            ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kEraseData:
            if(btn1_pressed)
            {
                current_state = StateId::kReturn;
                ui::GoToScreen(&display, StateId::kReturn); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kErasingInProgress;
                ui::GoToScreen(&display, StateId::kErasingInProgress);
                btn2_pressed = false;
            }
            break;
        case StateId::kErasingInProgress:
            // Starts erasing data - actually erases first sector (16*256 bytes) and set saved_pages_counter=0
            storage::EraseData();
            sleep_ms(3);

            current_state = StateId::kStandby;
            ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kReturn:
            if(btn1_pressed)
            {
                current_state = StateId::kReadData;
                ui::GoToScreen(&display, StateId::kReadData); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(&display, StateId::kStandby);
                btn2_pressed = false;
            }
            break;
        default:
            break;
        }
#endif
    }
}
