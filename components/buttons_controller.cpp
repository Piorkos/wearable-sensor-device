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
}

bool ButtonsController::btn1_pressed = false;
bool ButtonsController::btn2_pressed = false;
bool ButtonsController::irq_btn_1_enabled = false;
bool ButtonsController::irq_btn_2_enabled = false;

void ButtonsController::UpdateButtons(bool first_enable, bool second_enable, std::string first_label, std::string second_label)
{
    printf("ui::UpdateButtons: %b, %b \n", first_enable, second_enable);
    gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
    gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_FALL, false, ButtonCallback);
    irq_btn_1_enabled = first_enable;
    irq_btn_2_enabled = second_enable;
    add_alarm_in_ms(500, EnableButtons, nullptr, false);
    // add_alarm_in_ms(500, EnableButtons, &irq_state, false);
}

static void ButtonsController::ButtonCallback(uint gpio, uint32_t events)
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

static int64_t ButtonsController::EnableButtons(alarm_id_t id, void *irq_state)
{
    gpio_set_irq_enabled_with_callback(config::kButton_left_pin, GPIO_IRQ_EDGE_FALL, irq_btn_1_enabled, ButtonCallback);
    gpio_set_irq_enabled_with_callback(config::kButton_right_pin, GPIO_IRQ_EDGE_FALL, irq_btn_2_enabled, ButtonCallback);
    
    return 0;
}