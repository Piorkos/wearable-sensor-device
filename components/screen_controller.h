#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include <string.h>
#include "../core/config.h"
#include "../drivers/display/sharp-mip/display.h"
#include "../drivers/display/sharp-mip/sharp-mip/fonts/font_8x10.h"
#include "../drivers/display/sharp-mip/sharp-mip/fonts/font_16x20.h"
#include "../drivers/display/sharp-mip/sharp-mip/fonts/font_24x30.h"

class ScreenController
{
public:
    ScreenController(Display* display);
    
    void ShowOnScreen(std::string btn1_label, std::string btn2_label, std::string text_1, std::string text_2 = "");
    void RefreshTrackingScreen(std::string text_1, std::string text_2 = "");
    void ShowError(std::string error_msg);


private:
    Display* display_;
    uint16_t line_1_y;
    uint16_t line_2_y;
    uint16_t btn_1_y;
    uint16_t btn_1_x;   //in BYTES
    uint16_t btn_2_y;
    uint16_t btn_2_x;   //in BYTES
    uint16_t error_x;   //in BYTES
    uint16_t error_y;
};


#endif // SCREEN_CONTROLLER_H