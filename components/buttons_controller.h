#ifndef BUTTONS_CONTROLLER_H
#define BUTTONS_CONTROLLER_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "../core/config.h"



class ButtonsController
{
public:
    ButtonsController();

    void UpdateButtons(bool first_enable, bool second_enable, std::string first_label, std::string second_label);

    static bool btn1_pressed;
    static bool btn2_pressed;

private:

    static void ButtonCallback(uint gpio, uint32_t events);
    
    static int64_t EnableButtons(alarm_id_t id, void *irq_state);

    static bool irq_btn_1_enabled;
    static bool irq_btn_2_enabled;
};


#endif // BUTTONS_CONTROLLER_H