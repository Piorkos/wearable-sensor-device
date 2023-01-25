#ifndef UI_H
#define UI_H

#include "hardware/gpio.h"
#include "data.h"


void ButtonCallback(uint gpio, uint32_t events);

namespace ui
{
    void InitButtons();
    void UpdateSide(pico_ssd1306::SSD1306* display, std::string side_text_0, std::string side_text_1);
    void GoToScreen(pico_ssd1306::SSD1306* display, StateId screen_id);
    void UpdateButtons(bool top, bool bottom);
    int64_t EnableButtons(alarm_id_t id, void *user_data);

    struct ButtonsState
    {
        bool top{};
        bool bottom{};
    };
    

    void InitButtons()
    {
        printf("UI::InitButtons \n");
        gpio_init(config::kButton_top_pin);
        gpio_set_dir(config::kButton_top_pin, GPIO_IN);
        gpio_pull_up(config::kButton_top_pin);
        gpio_init(config::kButton_bottom_pin);
        gpio_set_dir(config::kButton_bottom_pin, GPIO_IN);
        gpio_pull_up(config::kButton_bottom_pin);
    }

    void GoToScreen(pico_ssd1306::SSD1306* display, StateId screen_id)
    {
        printf("UI::GoToScreen %i \n", screen_id);
        switch (screen_id)
        {
        case kInit:
            UpdateButtons(false, false);
            display->clear();
            drawText(display, font_12x16, "INIT...", 0, 10);
            display->sendBuffer();
            break;
        case kStandby:
            UpdateButtons(false, true);
            display->clear();
            drawText(display, font_12x16, "START", 20, 20);
            drawText(display, font_12x16, "TRAINING?", 0, 40);
            UpdateSide(display, " ", "YES");
            display->sendBuffer();
            break;
        case kGpsSearch:
            UpdateButtons(false, true);
            display->clear();
            drawText(display, font_12x16, "GPS:", 20, 10);
            drawText(display, font_12x16, "SEARCHING", 8, 30);
            UpdateSide(display, " ", "CNL");
            display->sendBuffer();
            break;
        case kGpsReady:
            UpdateButtons(true, true);
            display->clear();
            drawText(display, font_12x16, "GPS:", 20, 10);
            drawText(display, font_12x16, "READY", 18, 30);
            UpdateSide(display, "GO", "CNL");
            display->sendBuffer();
            break;
        case kTraining:

            break;
        case kStopTraining:

            break;
        
        default:
            break;
        }
    }

    void UpdateSide(pico_ssd1306::SSD1306* display, std::string side_text_0, std::string side_text_1)
    {
        printf("ui:UpdateSide: %s, %s \n", side_text_0.c_str(), side_text_1.c_str());
        drawText(display, font_8x8, &(side_text_0[0]), (config::kWidth - config::kSideFontSize), 0);
        drawText(display, font_8x8, &(side_text_0[1]), (config::kWidth - config::kSideFontSize), 9);
        drawText(display, font_8x8, &(side_text_0[2]), (config::kWidth - config::kSideFontSize), 18);
        drawText(display, font_8x8, &(side_text_1[0]), (config::kWidth - config::kSideFontSize), config::kHeight - 3*(config::kSideFontSize + 1));
        drawText(display, font_8x8, &(side_text_1[1]), (config::kWidth - config::kSideFontSize), config::kHeight - 2*(config::kSideFontSize + 1));
        drawText(display, font_8x8, &(side_text_1[2]), (config::kWidth - config::kSideFontSize), config::kHeight - config::kSideFontSize);
    }

    void UpdateButtons(bool top, bool bottom)
    {
        printf("ui::UpdateButtons: %b, %b \n", top, bottom);
        gpio_set_irq_enabled_with_callback(config::kButton_top_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
        gpio_set_irq_enabled_with_callback(config::kButton_bottom_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
        ButtonsState buttons_state{top, bottom};
        add_alarm_in_ms(1000, EnableButtons, &buttons_state, false);
    }

    int64_t EnableButtons(alarm_id_t id, void *buttons_state)
    {
        printf("ui::EnableButtons \n");
        gpio_set_irq_enabled_with_callback(config::kButton_top_pin, GPIO_IRQ_EDGE_FALL, static_cast<ButtonsState*>(buttons_state)->top, ButtonCallback);
        gpio_set_irq_enabled_with_callback(config::kButton_bottom_pin, GPIO_IRQ_EDGE_FALL, static_cast<ButtonsState*>(buttons_state)->bottom, ButtonCallback);

        return 0;
    }
}

#endif  // UI_H