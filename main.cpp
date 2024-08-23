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
#include "drivers/display/display.h"    // Sharp MIP
#include "drivers/display/sharp_mip_display.h"    // Sharp MIP
#include "drivers/lsm6dsox.h"  // onboard's gyro, accel
#include "utils/distance.h"
#include "utils/ui.h"           // controls UI
#include "utils/data.h"         // pins, parameters,...
#include "utils/sensors_data.h" // Struct to hold data from sensors


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
    for(int i = 0; i < 6; ++i)
    {
        printf("waiting %i \n", i);
        sleep_ms(1000);
    }

    #ifdef ARDUINO_NANO_RP2040
        printf("- ARDUINO : v1.5 \n");
    #else
        printf("- Pi Pico \n");
    #endif

    // SPI for Sharp MIP display
    spi_init(spi1, 2000000);
    spi_set_format( spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(config::kSPI_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(config::kSPI_sck_pin, GPIO_FUNC_SPI);

    gpio_init(config::kSPI_cs_pin);
    gpio_set_dir(config::kSPI_cs_pin, GPIO_OUT);
    gpio_put(config::kSPI_cs_pin, 0);  // this display is low on inactive
    sleep_ms(10);

    ui::InitDisplay();




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

    // SPI for Sharp MIP display
    spi_init(spi1, 2000000);
    spi_set_format( spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(config::kSPI_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(config::kSPI_sck_pin, GPIO_FUNC_SPI);

    gpio_init(config::kSPI_cs_pin);
    gpio_set_dir(config::kSPI_cs_pin, GPIO_OUT);
    gpio_put(config::kSPI_cs_pin, 0);  // this display is low on inactive
    sleep_ms(10);
    

    StateId current_state{StateId::kInit};
    ui::InitButtons();
    ui::GoToScreen(current_state);
    printf("Sharp MIP initialized \n");

    // // ---UI
    // pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(i2c1, config::kOledAddress, pico_ssd1306::Size::W128xH64);
    // StateId current_state{StateId::kInit};
    // ui::InitButtons();
    // printf("UI initialized \n");
    // ui::GoToScreen(&display, current_state);

    // ---IMU LSM6DSOX
    Imu imu{i2c0};
    printf("IMU initialized \n");

    // --- COMPASS LSM303D
    std::string compass_coordinates{};
    Compass compass;
    printf("COMPASS initialized \n");

    // ---GPS PA1010D
    hw_write_masked(&i2c1->hw->sda_hold, 5, I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(config::kI2C_0_sda_pin, config::kI2C_0_scl_pin, GPIO_FUNC_I2C));
    GPS gps;

    // ---Storage
    Storage storage;

    // ---timer
    struct repeating_timer timer_1;
    add_repeating_timer_ms(100, timer_callback, NULL, &timer_1);


    SensorsData sensors_data{};
    
    std::string data_to_store{"|"};


    while(true)
    {
        // printf("while current state = %i \n", current_state);
        
#ifdef I2C_CONNECTION_TEST
        
        int response{0};
        std::string msg_0{"COMP-"};
        std::string msg_1{"GPS-"};
        std::string msg_2{"IMU-"};

        response = compass.TestConnection();
        if(response == -1){
            msg_0.append("timeout");
        }else if(response == -2){
            msg_0.append("conn");
        }
        response = gps.TestConnection(i2c1);
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

        // sharp_mip::ToggleVCOM();
        // sleep_ms(100);

        switch (current_state)
        {
        case StateId::kInit:
            sleep_ms(1000);
            current_state = StateId::kStandby;
            ui::GoToScreen(StateId::kStandby);
            // ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kStandby:
            if(btn1_pressed)
            {
                current_state = StateId::kReadData;
                ui::GoToScreen(StateId::kReadData);
                // ui::GoToScreen(&display, StateId::kReadData);
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kGpsSearch;
                ui::GoToScreen(StateId::kGpsSearch);
                // ui::GoToScreen(&display, StateId::kGpsSearch);
                // TODO turn on GPS, get connection
                btn2_pressed = false;
            }
            break;
        case StateId::kGpsSearch:
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(StateId::kStandby);
                // ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn2_pressed = false;
            }
            if(read_sensors_flag)
            {
                read_sensors_flag = false;
                int has_fix{false};
                has_fix = gps.HasFix(sensors_data);
                if(has_fix)
                {
                    no_activity_counter = 0;
                    current_state = StateId::kGpsReady;
                    ui::GoToScreen(StateId::kGpsReady);
                    // ui::GoToScreen(&display, StateId::kGpsReady);
                }
            }
            break;
        case StateId::kGpsReady:
            if(btn1_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(StateId::kStandby);
                // ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                storage.AddNewTrainingMark();
                current_state = StateId::kTraining;
                ui::GoToScreen(StateId::kTraining);
                // ui::GoToScreen(&display, StateId::kTraining);
                // TODO turn on SENSORS and set up storing in FLASH
                btn2_pressed = false;
            }
            if(no_activity_counter > 300)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(StateId::kStandby);
                // ui::GoToScreen(&display, StateId::kStandby);
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
                compass.Read(sensors_data);
                // -- gyro, accel - LSM6DSOX
                imu.ReadAccelerometer(sensors_data);
                imu.ReadGyroscope(sensors_data);
                // -- gps & disntace
                int gps_error{0};
                bool gps_data = gps.ReadData1Per10(sensors_data, gps_error);
                if (gps_error < 0)
                {
                    error_msg.append(":GPS");
                    // TODO maybe it would be good to reset GPS here
                }
                sensors_data.distance = 0;
                if(gps_data)
                {
                    distance::CalculateDistance(sensors_data);
                }
                // ---write data to FLASH
                int success = storage.UpdateDataToStore(sensors_data, gps_data);
                printf("MAIN::training: UpdateDataToStore result = %i \n", success);

                ui::UpdateTraining(sensors_data.distance, error_msg);
                // ui::UpdateTraining(&display, sensors_data.distance, error_msg);
            }
            
            if(btn2_pressed)
            {
                no_activity_counter = 0;
                current_state = StateId::kStopTraining;
                ui::GoToScreen(StateId::kStopTraining);
                // ui::GoToScreen(&display, StateId::kStopTraining);
                btn2_pressed = false;
            }
            break;
        case StateId::kStopTraining:
            if(btn1_pressed)
            {
                current_state = StateId::kTraining;
                ui::GoToScreen(StateId::kTraining);
                // ui::GoToScreen(&display, StateId::kTraining);
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(StateId::kStandby);
                // ui::GoToScreen(&display, StateId::kStandby);
                // TODO turn off GPS
                btn2_pressed = false;
            }
            if(no_activity_counter > 300)
            {
                current_state = StateId::kTraining;
                ui::GoToScreen(StateId::kTraining);
                // ui::GoToScreen(&display, StateId::kTraining);
            }
            break;
        case StateId::kReadData:
            if(btn1_pressed)
            {
                current_state = StateId::kEraseData;
                ui::GoToScreen(StateId::kEraseData); 
                // ui::GoToScreen(&display, StateId::kEraseData); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kReadingInProgress;
                ui::GoToScreen(StateId::kReadingInProgress);
                // ui::GoToScreen(&display, StateId::kReadingInProgress);
                btn2_pressed = false;
            }
            
            break;
        case StateId::kReadingInProgress:
            // Starts reading data and sending via UART to PC
            storage.ReadAllData();
            sleep_ms(3);

            current_state = StateId::kStandby;
            ui::GoToScreen(StateId::kStandby);
            // ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kEraseData:
            if(btn1_pressed)
            {
                current_state = StateId::kReturn;
                ui::GoToScreen(StateId::kReturn); 
                // ui::GoToScreen(&display, StateId::kReturn); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kErasingInProgress;
                ui::GoToScreen(StateId::kErasingInProgress);
                // ui::GoToScreen(&display, StateId::kErasingInProgress);
                btn2_pressed = false;
            }
            break;
        case StateId::kErasingInProgress:
            // Starts erasing data - actually erases first sector (16*256 bytes) and set saved_pages_counter=0
            storage.EraseData();
            sleep_ms(3000);

            current_state = StateId::kStandby;
            ui::GoToScreen(StateId::kStandby);
            // ui::GoToScreen(&display, StateId::kStandby);
            break;
        case StateId::kReturn:
            if(btn1_pressed)
            {
                current_state = StateId::kReadData;
                ui::GoToScreen(StateId::kReadData); 
                // ui::GoToScreen(&display, StateId::kReadData); 
                btn1_pressed = false;
            }
            if(btn2_pressed)
            {
                current_state = StateId::kStandby;
                ui::GoToScreen(StateId::kStandby);
                // ui::GoToScreen(&display, StateId::kStandby);
                btn2_pressed = false;
            }
            break;
        default:
            break;
        }
#endif
    }

    return 0;
}
