#include "buttons_controller.h"

ButtonsController::ButtonsController()
{
        printf("ButtonsController Constructor \n");

        gpio_init(config::kButton_left_pin);
        gpio_set_dir(config::kButton_left_pin, GPIO_IN);
        gpio_pull_up(config::kButton_left_pin);
        gpio_init(config::kButton_right_pin);
        gpio_set_dir(config::kButton_right_pin, GPIO_IN);
        gpio_pull_up(config::kButton_right_pin);

        UpdateButtons(false, false);
}

bool ButtonsController::btn1_pressed_ = false;
bool ButtonsController::btn2_pressed_ = false;
bool ButtonsController::irq_btn_1_enabled_ = false;
bool ButtonsController::irq_btn_2_enabled_ = false;

bool ButtonsController::IsBtn1Pressed()
{
    return btn1_pressed_;
}
bool ButtonsController::IsBtn2Pressed()
{
    return btn2_pressed_;
}

void ButtonsController::UpdateButtons(bool first_enable, bool second_enable)
{
    btn1_pressed_ = false;
    btn2_pressed_ = false;
    printf("ui::UpdateButtons: %b, %b \n", first_enable, second_enable);
    gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_RISE, false, ButtonCallback);
    gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_RISE, false, ButtonCallback);
    irq_btn_1_enabled_ = first_enable;
    irq_btn_2_enabled_ = second_enable;
    add_alarm_in_ms(500, EnableButtons, nullptr, false);
}

void ButtonsController::ButtonCallback(uint gpio, uint32_t events)
{
    printf("ButtonCallback \n");
    if(events == GPIO_IRQ_EDGE_RISE)
    {
        if(gpio == config::kButton_left_pin)
        {
            printf("ButtonCallback BTN 1 \n");
            btn1_pressed_ = true;
        }
        if(gpio == config::kButton_right_pin)
        {
            printf("ButtonCallback BTN 2 \n");
            btn2_pressed_ = true;
        }
    }
}

int64_t ButtonsController::EnableButtons(alarm_id_t id, void *irq_state)
{
    gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_RISE, irq_btn_1_enabled_, ButtonCallback);
    gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_RISE, irq_btn_2_enabled_, ButtonCallback);
    
    return 0;
}