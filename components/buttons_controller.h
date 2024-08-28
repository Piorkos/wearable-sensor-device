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

    bool IsBtn1Pressed();
    bool IsBtn2Pressed();
    void UpdateButtons(bool first_enable, bool second_enable);

private:
    static void ButtonCallback(uint gpio, uint32_t events);
    static int64_t EnableButtons(alarm_id_t id, void *irq_state);

    static bool btn1_pressed_;
    static bool btn2_pressed_;
    static bool irq_btn_1_enabled_;
    static bool irq_btn_2_enabled_;
};


#endif // BUTTONS_CONTROLLER_H