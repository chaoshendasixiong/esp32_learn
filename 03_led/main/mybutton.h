#ifndef __h_my_button_h__
#define __h_my_button_h__
#include <stdint.h>

void myButton_init(uint32_t gpio) {
    gpio_pad_select_gpio(gpio);//选择一个GPIO
    gpio_set_direction(gpio, GPIO_MODE_INPUT);//把这个GPIO作为输出


    
}


uint32_t myButton_state(uint32_t gpio) {
    return gpio_get_level(gpio);
}





#endif