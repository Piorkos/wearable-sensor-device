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
#include "core/distance.h"
// #include "core/ui.h"           // controls UI
#include "core/config.h"         // pins, parameters,...
#include "core/helpers.h"         // helper functions
#include "core/sensors_data.h" // Struct to hold data from sensors
#include "components/buttons_controller.h"
#include "components/screen_controller.h"



// *************************
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

// void ButtonCallback(uint gpio, uint32_t events)
// {
//     printf("ButtonCallback \n");
//     if(events == GPIO_IRQ_EDGE_FALL)
//     {
//         if(gpio == config::kButton_left_pin)
//         {
//             printf("ButtonCallback BTN 1 \n");
//             btn1_pressed = true;
//         }
//         if(gpio == config::kButton_right_pin)
//         {
//             printf("ButtonCallback BTN 2 \n");
//             btn2_pressed = true;
//         }
//     }
// }


int main() {

    stdio_init_all();

    // ---wait for connection to CoolTerm on Mac
    for(int i = 0; i < 2; ++i)
    {
        printf("waiting %i \n", i);
        sleep_ms(1000);
    }

    #ifdef ARDUINO_NANO_RP2040
        printf("- ARDUINO : v1.5 \n");
    #else
        printf("- Pi Pico \n");
    #endif



    // *************************
    // ------Init commutioncation with peripherals
    InitI2C();
    InitSPI();
    

    // ---Screen
    ScreenController screen;

    // ---Buttons
    ButtonsController buttons;

    StateId current_state{StateId::kInit};
    buttons.UpdateButtons(false, false, "", "");
    screen.ShowOnScreen(current_state);

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
        

        // sharp_mip::ToggleVCOM();
        // sleep_ms(100);

        switch (current_state)
        {
        case StateId::kInit:
            sleep_ms(1000);
            current_state = StateId::kStandby;
            buttons.UpdateButtons(true, true, "OPT", "YES");
            screen.ShowOnScreen(StateId::kStandby);
            break;
        case StateId::kStandby:
            if(buttons.btn1_pressed)
            {
                current_state = StateId::kReadData;
                buttons.UpdateButtons(true, true, "NO", "YES");
                screen.ShowOnScreen(StateId::kReadData);
                buttons.btn1_pressed = false;
            }
            if(buttons.btn2_pressed)
            {
                current_state = StateId::kGpsSearch;
                buttons.UpdateButtons(false, true, "", "YES");
                screen.ShowOnScreen(StateId::kGpsSearch);
                // TODO turn on GPS, get connection
                buttons.btn2_pressed = false;
            }
            break;
        // case StateId::kGpsSearch:
        //     if(buttons.btn2_pressed)
        //     {
        //         current_state = StateId::kStandby;
        //         screen.ShowOnScreen(StateId::kStandby);
        //         // TODO turn off GPS
        //         buttons.btn2_pressed = false;
        //     }
        //     if(read_sensors_flag)
        //     {
        //         read_sensors_flag = false;
        //         int has_fix{false};
        //         has_fix = gps.HasFix(sensors_data);
        //         if(has_fix)
        //         {
        //             no_activity_counter = 0;
        //             current_state = StateId::kGpsReady;
        //             screen.ShowOnScreen(StateId::kGpsReady);
        //         }
        //     }
        //     break;
        // case StateId::kGpsReady:
        //     if(btn1_pressed)
        //     {
        //         current_state = StateId::kStandby;
        //         screen.ShowOnScreen(StateId::kStandby);
        //         // TODO turn off GPS
        //         btn1_pressed = false;
        //     }
        //     if(btn2_pressed)
        //     {
        //         storage.AddNewTrainingMark();
        //         current_state = StateId::kTraining;
        //         screen.ShowOnScreen(StateId::kTraining);
        //         // TODO turn on SENSORS and set up storing in FLASH
        //         btn2_pressed = false;
        //     }
        //     if(no_activity_counter > 300)
        //     {
        //         current_state = StateId::kStandby;
        //         screen.ShowOnScreen(StateId::kStandby);
        //     }
        //     break;
        // case StateId::kTraining:
        //     // printf("StateId::kTraining \n");
        //     if(read_sensors_flag)
        //     {
        //         printf("StateId::kTraining read_sensors_flag \n");
        //         read_sensors_flag = false;
        //         std::string error_msg{""};

        //         // -- compass - LSM303D
        //         compass.Read(sensors_data);
        //         // -- gyro, accel - LSM6DSOX
        //         imu.ReadAccelerometer(sensors_data);
        //         imu.ReadGyroscope(sensors_data);
        //         // -- gps & disntace
        //         int gps_error{0};
        //         bool gps_data = gps.ReadData1Per10(sensors_data, gps_error);
        //         if (gps_error < 0)
        //         {
        //             error_msg.append(":GPS");
        //             // TODO maybe it would be good to reset GPS here
        //         }
        //         sensors_data.distance = 0;
        //         if(gps_data)
        //         {
        //             distance::CalculateDistance(sensors_data);
        //         }
        //         // ---write data to FLASH
        //         int success = storage.UpdateDataToStore(sensors_data, gps_data);
        //         printf("MAIN::training: UpdateDataToStore result = %i \n", success);

        //         ui::UpdateTraining(sensors_data.distance, error_msg);
        //     }
            
        //     if(btn2_pressed)
        //     {
        //         no_activity_counter = 0;
        //         current_state = StateId::kStopTraining;
        //         screen.ShowOnScreen(StateId::kStopTraining);
        //         btn2_pressed = false;
        //     }
        //     break;
        // case StateId::kStopTraining:
        //     if(btn1_pressed)
        //     {
        //         current_state = StateId::kTraining;
        //         screen.ShowOnScreen(StateId::kTraining);
        //         btn1_pressed = false;
        //     }
        //     if(btn2_pressed)
        //     {
        //         current_state = StateId::kStandby;
        //         screen.ShowOnScreen(StateId::kStandby);
        //         // TODO turn off GPS
        //         btn2_pressed = false;
        //     }
        //     if(no_activity_counter > 300)
        //     {
        //         current_state = StateId::kTraining;
        //         screen.ShowOnScreen(StateId::kTraining);
        //     }
        //     break;
        // case StateId::kReadData:
        //     if(btn1_pressed)
        //     {
        //         current_state = StateId::kEraseData;
        //         screen.ShowOnScreen(StateId::kEraseData);
        //         btn1_pressed = false;
        //     }
        //     if(btn2_pressed)
        //     {
        //         current_state = StateId::kReadingInProgress;
        //         screen.ShowOnScreen(StateId::kReadingInProgress);
        //         btn2_pressed = false;
        //     }
            
        //     break;
        // case StateId::kReadingInProgress:
        //     // Starts reading data and sending via UART to PC
        //     storage.ReadAllData();
        //     sleep_ms(3);

        //     current_state = StateId::kStandby;
        //     screen.ShowOnScreen(StateId::kStandby);
        //     break;
        // case StateId::kEraseData:
        //     if(btn1_pressed)
        //     {
        //         current_state = StateId::kReturn;
        //         screen.ShowOnScreen(StateId::kReturn);
        //         btn1_pressed = false;
        //     }
        //     if(btn2_pressed)
        //     {
        //         current_state = StateId::kErasingInProgress;
        //         screen.ShowOnScreen(StateId::kErasingInProgress);
        //         btn2_pressed = false;
        //     }
        //     break;
        // case StateId::kErasingInProgress:
        //     storage.EraseData();
        //     sleep_ms(3000);

        //     current_state = StateId::kStandby;
        //     screen.ShowOnScreen(StateId::kStandby);
        //     break;
        // case StateId::kReturn:
        //     if(btn1_pressed)
        //     {
        //         current_state = StateId::kReadData;
        //         screen.ShowOnScreen(StateId::kReadData);
        //         btn1_pressed = false;
        //     }
        //     if(btn2_pressed)
        //     {
        //         current_state = StateId::kStandby;
        //         screen.ShowOnScreen(StateId::kStandby);
        //         btn2_pressed = false;
        //     }
        //     break;
        // default:
        //     break;
        }
    }

    return 0;
}
