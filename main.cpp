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

#include "drivers/lsm303d.h"            // compass
#include "drivers/storage.h"            // onboard Flesh memory
#include "drivers/pa1010d.h"            // GPS
#include "drivers/display/display.h"    // Sharp MIP
#include "drivers/display/sharp_mip_display.h"    // Sharp MIP
#include "drivers/lsm6dsox.h"  // onboard's gyro, accel
#include "core/distance.h"
#include "core/config.h"         // pins, parameters,...
#include "core/helpers.h"         // helper functions
#include "core/sensors_data.h" // Struct to hold data from sensors
#include "components/buttons_controller.h"
#include "components/screen_controller.h"
#include "components/tracker.h"



// *************************
bool tick_100_ms{false};
bool timer_callback(struct repeating_timer *timer_1)
{
    tick_100_ms = true;
    return true;
}



int main() {

    stdio_init_all();

    // ---wait for connection to CoolTerm on Mac
    for(int i = 0; i < 5; ++i)
    {
        // printf("waiting %i \n", i);
        sleep_ms(1000);
    }


    // *************************
    // ------Init commutioncation with peripherals
    InitI2C();
    InitSPI();

    // ------Init peripherals
    // ---Screen
    Display* display = new SharpMipDisplay(config::kWidth, config::kHeight, spi1, config::kSPI_cs_pin);
    ScreenController screen(display);
    // ---Buttons
    ButtonsController buttons;
    // ---GPS
    GPS gps;
    // ---Storage
    Storage storage;
    // ---Tracker
    Tracker* tracker = new Tracker{screen, storage, gps, i2c0, i2c1};

    // ---timer
    struct repeating_timer timer_1;
    add_repeating_timer_ms(100, timer_callback, NULL, &timer_1);


    StateId current_state{StateId::kInit};

    while(true)
    {
        switch (current_state)
        {
        case StateId::kInit:
            sleep_ms(1000);
            current_state = StateId::kStandby;
            screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
            buttons.UpdateButtons(true, true);
            break;
        case StateId::kStandby:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kReadData;
                screen.ShowOnScreen("NO", "YES", "READ", "DATA?");
                buttons.UpdateButtons(true, true);
            }
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kGpsSearch;
                screen.ShowOnScreen("", "CNL", "GPS", "SEARCHING");
                buttons.UpdateButtons(false, true);
            }
            break;
        case StateId::kGpsSearch:
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kStandby;
                screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
                buttons.UpdateButtons(true, true);
            }
            if(tick_100_ms) //execute only if 100 ms passed since last execution
            {
                tick_100_ms = false;
                int has_fix{false};
                has_fix = gps.HasFix();
                if(has_fix)
                {
                    current_state = StateId::kGpsReady;
                    screen.ShowOnScreen("CNL", "GO", "GPS", "READY");
                    buttons.UpdateButtons(true, true);
                }
            }
            break;
        case StateId::kGpsReady:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kStandby;
                screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
                buttons.UpdateButtons(true, true);
                // TODO turn off GPS
            }
            if(buttons.IsBtn2Pressed())
            {
                storage.AddNewTrainingMark();
                current_state = StateId::kTraining;
                tracker->Reset();
                screen.ShowOnScreen("", "END", "0:00");
                buttons.UpdateButtons(false, true);
            }
            break;
        case StateId::kTraining:
            if(tick_100_ms)
            {
                // printf("StateId::kTraining tick_100_ms \n");
                tick_100_ms = false;
                tracker->Update();
            }
            
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kStopTraining;
                screen.ShowOnScreen("NO", "YES", "END", "TRACKING?");
                buttons.UpdateButtons(true, true);
            }
            break;
        case StateId::kStopTraining:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kTraining;
                screen.ShowOnScreen("", "END", "0:00");
                buttons.UpdateButtons(false, true);
            }
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kStandby;
                screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
                buttons.UpdateButtons(true, true);
            }
            break;
        case StateId::kReadData:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kEraseData;
                screen.ShowOnScreen("NO", "YES", "ERASE", "DATA?");
                buttons.UpdateButtons(true, true);
            }
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kReadingInProgress;
                screen.ShowOnScreen("", "", "READING");
                buttons.UpdateButtons(false, false);
            }
            break;
        case StateId::kReadingInProgress:
            // Starts reading data and sending via UART to PC
            storage.ReadAllData();
            sleep_ms(3);

            current_state = StateId::kStandby;
            screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
            buttons.UpdateButtons(true, true);
            break;
        case StateId::kEraseData:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kReturn;
                screen.ShowOnScreen("NO", "YES", "HOME", "SCREEN?");
                buttons.UpdateButtons(true, true);
            }
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kErasingInProgress;
                screen.ShowOnScreen("", "", "ERASING");
                buttons.UpdateButtons(false, false);
            }
            break;
        case StateId::kErasingInProgress:
            storage.EraseData();
            sleep_ms(3000);     // Delay, so the user has time to read the message "erasing". Otherwise it would be not clear if erasing happened or not.

            current_state = StateId::kStandby;
            screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
            buttons.UpdateButtons(true, true);
            break;
        case StateId::kReturn:
            if(buttons.IsBtn1Pressed())
            {
                current_state = StateId::kReadData;
                screen.ShowOnScreen("NO", "YES", "READ", "DATA?");
                buttons.UpdateButtons(true, true);
            }
            if(buttons.IsBtn2Pressed())
            {
                current_state = StateId::kStandby;
                screen.ShowOnScreen("OPT", "YES", "START", "TRACKING?");
                buttons.UpdateButtons(true, true);
            }
            break;
        default:
            break;
        }
    }

    return 0;
}
