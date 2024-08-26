#ifndef UI_H
#define UI_H

#include "hardware/gpio.h"
#include "config.h"

// #include "../drivers/display/write.h"    // Sharp MIP
#include "../drivers/display/display.h"             // Sharp MIP
#include "../drivers/display/sharp_mip_display.h"   // Sharp MIP
#include "../drivers/display/font_12x16.h"   // Sharp MIP
#include "../drivers/display/font_16x20.h"   // Sharp MIP


void ButtonCallback(uint gpio, uint32_t events);

namespace ui
{
    struct ButtonsState
    {
        bool left{};
        bool right{};
    };
    
    ButtonsState buttons_state{false, false};
    // --- trainig-duration
    uint8_t second_counter{0};
    uint8_t seconds{0};
    uint8_t minutes{0};
    uint8_t hours{0};
    int run_distance{0};       // distance for full trainig
    SharpMipDisplay* display = new SharpMipDisplay(config::kWidth, config::kHeight, spi1, config::kSPI_cs_pin);


    
    void InitButtons();
    // Sharp MIP
    void InitDisplay();
    void UpdateSide(std::string side_text_0, std::string side_text_1);
    void GoToScreen(StateId screen_id, std::string msg_0 = "", std::string msg_1 = "", std::string msg_2 = "");
    void UpdateTraining(int distance, std::string error_msg);
    // // OLED
    // void UpdateSide(pico_ssd1306::SSD1306* display, std::string side_text_0, std::string side_text_1);
    // void GoToScreen(pico_ssd1306::SSD1306* display, StateId screen_id, std::string msg_0 = "", std::string msg_1 = "", std::string msg_2 = "");
    // void UpdateTraining(pico_ssd1306::SSD1306* display, int distance, std::string error_msg);
    // Common for both types of display
    void UpdateButtons(bool left, bool right);
    int64_t EnableButtons(alarm_id_t id, void *user_data);
    void ResetTraining();


    void InitButtons()
    {
        printf("UI::InitButtons \n");
        gpio_init(config::kButton_left_pin);
        gpio_set_dir(config::kButton_left_pin, GPIO_IN);
        gpio_pull_up(config::kButton_left_pin);
        gpio_init(config::kButton_right_pin);
        gpio_set_dir(config::kButton_right_pin, GPIO_IN);
        gpio_pull_up(config::kButton_right_pin);
    }

    void InitDisplay()
    {
        sleep_ms(1000);
        display->ClearScreen();
    }

    // -- Sharp MIP

    void GoToScreen(StateId screen_id, std::string msg_0, std::string msg_1, std::string msg_2)
    {
        printf("UI::GoToScreen %s \n", state_id_names[screen_id].c_str());
        display->ClearScreen();
        switch (screen_id)
        {
        case kInit:
            UpdateButtons(false, false);
            display->DrawLineOfText(0, 20, "INIT:::", kFont_12_16);
            display->RefreshScreen(0, 40);
            break;
        case kStandby:
            ResetTraining();
            UpdateButtons(true, true);
            display->DrawLineOfText(0, 20, "START", kFont_12_16);
            display->DrawLineOfText(0, 40, "TRACKING?", kFont_12_16);
            UpdateSide("OPT", "YES");
            display->RefreshScreen(20, 160);
            break;
        case kGpsSearch:
            UpdateButtons(false, true);
            display->DrawLineOfText(0, 20, "GPS", kFont_12_16);
            display->DrawLineOfText(0, 40, "SEARCHING", kFont_12_16);
            UpdateSide("", "CNL");
            display->RefreshScreen(0, 160);
            break;
        case kGpsReady:
            UpdateButtons(true, true);
            display->DrawLineOfText(0, 20, "GPS", kFont_12_16);
            display->DrawLineOfText(0, 40, "READY", kFont_12_16);
            UpdateSide("CNL", "GO");
            display->RefreshScreen(0, 160);
            break;
        case kTraining:
            UpdateButtons(false, true);
            // display->ClearScreen();
            // display->DrawLineOfText(0, 20, ".", kFont_12_16);
            // display->DrawLineOfText(0, 40, "TIME: ", kFont_16_20);
            // UpdateSide("", "END");
            // display->RefreshScreen(0, 160);
            break;
        case kStopTraining:
            UpdateButtons(true, true);            
            display->DrawLineOfText(0, 20, "END", kFont_12_16);
            display->DrawLineOfText(0, 40, "TRACKING?", kFont_12_16);
            UpdateSide("NO", "YES");
            display->RefreshScreen(0, 160);
            break;
        case kReadData:
            UpdateButtons(true, true);            
            display->DrawLineOfText(0, 20, "READ", kFont_12_16);
            display->DrawLineOfText(0, 40, "DATA?", kFont_12_16);
            UpdateSide("NO", "YES");
            display->RefreshScreen(0, 160);
            break;
        case kReadingInProgress:
            UpdateButtons(false, false);            
            display->DrawLineOfText(0, 20, "READING", kFont_12_16);
            display->DrawLineOfText(0, 40, ":::", kFont_12_16);
            UpdateSide("", "");
            display->RefreshScreen(0, 160);
            break;
        case kEraseData:
            UpdateButtons(true, true);            
            display->DrawLineOfText(0, 20, "ERASE", kFont_12_16);
            display->DrawLineOfText(0, 40, "DATA?", kFont_12_16);
            UpdateSide("NO", "YES");
            display->RefreshScreen(0, 160);
            break;
        case kErasingInProgress:
            UpdateButtons(false, false);            
            display->DrawLineOfText(0, 20, "ERASING", kFont_12_16);
            display->DrawLineOfText(0, 40, ":::", kFont_12_16);
            UpdateSide("", "");
            display->RefreshScreen(0, 160);
            break;
        case kReturn:
            UpdateButtons(true, true);            
            display->DrawLineOfText(0, 20, "HOME", kFont_12_16);
            display->DrawLineOfText(0, 40, "SCREEN?", kFont_12_16);
            UpdateSide("NO", "YES");
            display->RefreshScreen(0, 160);
            break;
        case kError:
            UpdateButtons(false, false);            
            display->DrawLineOfText(0, 0, msg_0, kFont_12_16);
            display->DrawLineOfText(0, 40, msg_1, kFont_12_16);
            display->DrawLineOfText(0, 60, msg_2, kFont_12_16);
            UpdateSide("", "");
            display->RefreshScreen(0, 160);
            break;
        
        default:
            break;
        }
    }


    /**
     * @brief Update time and distance displayed on the display.
     * 
     * @param display handler to display
     * @param distance new value of distance
     */
    void UpdateTraining(int distance, std::string error_msg)
    {
        printf("ui::UpdateTraining \n");

        // TODO display distance

        ++second_counter;
        std::string time{};

        if(second_counter == 10)
        {
            second_counter = 0;
            ++seconds;
            if(seconds == 60)
            {
                seconds = 0;
                ++minutes;
            }
            if(minutes == 60)
            {
                minutes = 0;
                ++hours;
            }
            // printf("time: min=%i, sec=%i \n", minutes, seconds);        
        }

        if(hours < 1)
        {
            if(seconds < 10)
            {
                time = std::to_string(minutes) + ":0" + std::to_string(seconds);
            }
            else
            {
                time = std::to_string(minutes) + ":" + std::to_string(seconds);
            }
        }
        else
        {
            if(minutes < 10)
            {
                time = std::to_string(hours) + ":0" + std::to_string(minutes);
            }
            else
            {
                time = std::to_string(hours) + ":" + std::to_string(minutes);
            }

        }

        run_distance += distance;
        std::string distance_s{std::to_string(run_distance) + " m"};

        // display->ClearScreen();
        if(error_msg != "")
        {
            std::string error_sentance = "E";
            error_sentance.append(error_msg);
            display->DrawLineOfText(0, 20, error_sentance, kFont_12_16);
            display->DrawLineOfText(0, 60, time, kFont_16_20);
        }
        else
        {
            display->DrawLineOfText(0, 60, time, kFont_16_20);
        }
        
        UpdateSide("", "END");

        display->RefreshScreen(20, 156);
    }

    void UpdateSide(std::string side_text_0, std::string side_text_1)
    {
        printf("ui:UpdateSide: %s, %s \n", side_text_0.c_str(), side_text_1.c_str());
        display->DrawLineOfText(0, 140, side_text_0, kFont_12_16);
        display->DrawLineOfText(10, 140, side_text_1, kFont_12_16);
    }


    // // --OLED

    // void GoToScreen(pico_ssd1306::SSD1306* display, StateId screen_id, std::string msg_0, std::string msg_1, std::string msg_2)
    // {
    //     printf("UI::GoToScreen %s \n", state_id_names[screen_id].c_str());
    //     switch (screen_id)
    //     {
    //     case kInit:
    //         UpdateButtons(false, false);
    //         display->clear();
    //         drawText(display, font_12x16, "INIT...", 0, 10);
    //         display->sendBuffer();
    //         break;
    //     case kStandby:
    //         ResetTraining();
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "START", 20, 0);
    //         drawText(display, font_12x16, "TRACKING?", 0, 20);
    //         UpdateSide(display, "OPTIONS", "YES");
    //         display->sendBuffer();
    //         break;
    //     case kGpsSearch:
    //         UpdateButtons(false, true);
    //         display->clear();
    //         drawText(display, font_12x16, "GPS:", 20, 0);
    //         drawText(display, font_12x16, "SEARCHING", 8, 20);
    //         UpdateSide(display, " ", "CANCEL");
    //         display->sendBuffer();
    //         break;
    //     case kGpsReady:
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "GPS:", 20, 0);
    //         drawText(display, font_12x16, "READY", 18, 20);
    //         UpdateSide(display, "CANCEL", "START");
    //         display->sendBuffer();
    //         break;
    //     case kTraining:
    //         UpdateButtons(false, true);
    //         display->clear();
    //         // drawText(display, font_12x16, "DISTANCE:", 20, 0);
    //         drawText(display, font_16x32, "TIME:", 2, 0);
    //         UpdateSide(display, " ", "END");
    //         display->sendBuffer();
    //         break;
    //     case kStopTraining:
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "END", 40, 0);
    //         drawText(display, font_12x16, "TRACKING?", 14, 20);
    //         UpdateSide(display, "NO", "YES");
    //         display->sendBuffer();
    //         break;
    //     case kReadData:
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "READ", 26, 0);
    //         drawText(display, font_12x16, "DATA?", 18, 20);
    //         UpdateSide(display, "NO", "YES");
    //         display->sendBuffer();
    //         break;
    //     case kReadingInProgress:
    //         UpdateButtons(false, false);
    //         display->clear();
    //         drawText(display, font_12x16, "READING", 20, 0);
    //         drawText(display, font_12x16, "...", 34, 20);
    //         display->sendBuffer();
    //         break;
    //     case kEraseData:
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "ERASE", 20, 0);
    //         drawText(display, font_12x16, "DATA?", 20, 20);
    //         UpdateSide(display, "NO", "YES");
    //         display->sendBuffer();
    //         break;
    //     case kErasingInProgress:
    //         UpdateButtons(false, false);
    //         display->clear();
    //         drawText(display, font_12x16, "ERASING", 20, 0);
    //         drawText(display, font_12x16, "...", 34, 20);
    //         display->sendBuffer();
    //         break;
    //     case kReturn:
    //         UpdateButtons(true, true);
    //         display->clear();
    //         drawText(display, font_12x16, "HOME", 26, 0);
    //         drawText(display, font_12x16, "SCREEN?", 18, 20);
    //         UpdateSide(display, "NO", "YES");
    //         display->sendBuffer();
    //         break;
    //     case kError:
    //         UpdateButtons(false, false);
    //         display->clear();
    //         drawText(display, font_12x16, msg_0.c_str(), 0, 10);
    //         drawText(display, font_12x16, msg_1.c_str(), 0, 26);
    //         drawText(display, font_12x16, msg_2.c_str(), 0, 42);
    //         UpdateSide(display, " ", " ");
    //         display->sendBuffer();
    //         break;
        
    //     default:
    //         break;
    //     }
    // }


    // /**
    //  * @brief Update time and distance displayed on the display.
    //  * 
    //  * @param display handler to display
    //  * @param distance new value of distance
    //  */
    // void UpdateTraining(pico_ssd1306::SSD1306* display, int distance, std::string error_msg)
    // {
    //     printf("ui::UpdateTraining \n");

    //     // TODO display distance

    //     ++second_counter;
    //     std::string time{};

    //     if(second_counter == 10)
    //     {
    //         second_counter = 0;
    //         ++seconds;
    //         if(seconds == 60)
    //         {
    //             seconds = 0;
    //             ++minutes;
    //         }
    //         if(minutes == 60)
    //         {
    //             minutes = 0;
    //             ++hours;
    //         }
    //         printf("time: min=%i, sec=%i \n", minutes, seconds);        
    //     }

    //     if(hours < 1)
    //     {
    //         if(seconds < 10)
    //         {
    //             time = std::to_string(minutes) + ":0" + std::to_string(seconds);
    //         }
    //         else
    //         {
    //             time = std::to_string(minutes) + ":" + std::to_string(seconds);
    //         }
    //     }
    //     else
    //     {
    //         if(minutes < 10)
    //         {
    //             time = std::to_string(hours) + ":0" + std::to_string(minutes);
    //         }
    //         else
    //         {
    //             time = std::to_string(hours) + ":" + std::to_string(minutes);
    //         }

    //     }

    //     run_distance += distance;

    //     std::string distance_s{std::to_string(run_distance) + " m"};
    //     display->clear();
    //     if(error_msg != "")
    //     {
    //         std::string error_sentance = "E";
    //         error_sentance.append(error_msg);
    //         drawText(display, font_12x16, error_sentance.c_str(), 0, 0);
    //         drawText(display, font_16x32, time.c_str(), 0, 20);
    //         // drawText(display, font_12x16, distance_s.c_str(), 0, 38);
    //     }
    //     else
    //     {
    //         drawText(display, font_16x32, time.c_str(), 16, 10);
    //         // drawText(display, font_12x16, distance_s.c_str(), 0, 20);
    //     }
    //     UpdateSide(display, " ", "END");
    //     display->sendBuffer();
    // }

    // void UpdateSide(pico_ssd1306::SSD1306* display, std::string side_text_0, std::string side_text_1)
    // {
    //     printf("ui:UpdateSide: %s, %s \n", side_text_0.c_str(), side_text_1.c_str());
    //     drawText(display, font_8x8, side_text_0.c_str(), 0, config::kHeight - config::kSideFontSize);
    //     drawText(display, font_8x8, side_text_1.c_str(), config::kWidth - (config::kWidth/2), config::kHeight - config::kSideFontSize);
    // }


    // --Common for both types of display

    void UpdateButtons(bool left, bool right)
    {
        printf("ui::UpdateButtons: %b, %b \n", left, right);
        gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
        gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
        buttons_state.left = left;
        buttons_state.right = right;
        add_alarm_in_ms(500, EnableButtons, &buttons_state, false);
    }

    int64_t EnableButtons(alarm_id_t id, void *buttons_state)
    {
        printf("ui::EnableButtons: %b, %b \n", static_cast<ButtonsState*>(buttons_state)->left, static_cast<ButtonsState*>(buttons_state)->right);
        gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_FALL, static_cast<ButtonsState*>(buttons_state)->left, ButtonCallback);
        gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_FALL, static_cast<ButtonsState*>(buttons_state)->right, ButtonCallback);

        return 0;
    }

    /**
     * @brief Sets all timer variables to initial values.
     * 
     */
    void ResetTraining()
    {
        distance::Reset();
        
        run_distance = 0;
        second_counter = 0;
        seconds = 0;
        minutes = 0;
        hours = 0;
    }
}

#endif  // UI_H