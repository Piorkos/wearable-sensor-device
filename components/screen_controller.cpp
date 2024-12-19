#include "screen_controller.h"

ScreenController::ScreenController(Display* display)
:display_{display}
{
    // printf("ScreenController Constructor \n");

    line_1_y = 22;
    line_2_y = 46;
    btn_1_x = 0;
    btn_1_y = config::kHeight - 17;
    btn_2_x = config::kWidth/8 - 7;
    btn_2_y = config::kHeight - 17;
    error_x = 0;
    error_y = 0;

    ShowOnScreen("", "", "INIT");
}

void ScreenController::ShowOnScreen(std::string btn1_label, std::string btn2_label, std::string text_1, std::string text_2)
{
    // printf("ScreenController::ShowOnScreen  \n");

    display_->ClearScreen();
    display_->DrawLineOfText(0, line_1_y, text_1, kFont_16_20);
    display_->DrawLineOfText(0, line_2_y, text_2, kFont_16_20);
    display_->DrawLineOfText(btn_1_x, btn_1_y, btn1_label, kFont_16_20);
    display_->DrawLineOfText(btn_2_x, btn_2_y, btn2_label, kFont_16_20, Display::Mode::kMix);
    display_->RefreshScreen(line_1_y, config::kHeight);
}

void ScreenController::RefreshTrackingScreen(std::string text_1, std::string text_2)
{
    printf("ScreenController::RefreshTrackingScreen  \n");

    display_->DrawLineOfText(0, line_1_y, text_1, kFont_24_30);
    display_->DrawLineOfText(0, line_2_y, text_2, kFont_16_20);
    display_->RefreshScreen(line_1_y, (line_2_y + 20));
}

void ScreenController::ShowError(std::string error_msg)
{
    display_->DrawLineOfText(error_x, error_y, error_msg, kFont_16_20);
    display_->RefreshScreen(error_y, (error_y + 20));
}