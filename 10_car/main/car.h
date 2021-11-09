#ifndef __h_car_l298n_h__
#define __h_car_l298n_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"


#define IN1_PIN   14
#define IN2_PIN   12
#define IN3_PIN   13
#define IN4_PIN   15

void carReset() {
    gpio_set_level(IN1_PIN, 0);
    gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0);
    gpio_set_level(IN4_PIN, 0);
}

void initL298N() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_OUTPUT;            
    io_conf.pin_bit_mask = (1ULL<<IN1_PIN)|(1ULL<<IN2_PIN)|(1ULL<<IN3_PIN)|(1ULL<<IN4_PIN);  
    io_conf.pull_down_en = 0;  
    io_conf.pull_up_en = 0;     
    gpio_config(&io_conf);  
    carReset();
    vTaskDelay(100 / portTICK_RATE_MS);
}

#define FORWARD {                                            \
    gpio_set_level(IN1_PIN, 1);gpio_set_level(IN2_PIN, 0);   \
    gpio_set_level(IN3_PIN, 1);gpio_set_level(IN4_PIN, 0);   \
}
#define BACK {                                            \
    gpio_set_level(IN1_PIN, 0);gpio_set_level(IN2_PIN, 1);   \
    gpio_set_level(IN3_PIN, 0);gpio_set_level(IN4_PIN, 1);   \
}
#define LEFT {                                            \
    gpio_set_level(IN1_PIN, 1);gpio_set_level(IN2_PIN, 0);   \
    gpio_set_level(IN3_PIN, 0);gpio_set_level(IN4_PIN, 1);   \
}
#define RIGHT {                                            \
    gpio_set_level(IN1_PIN, 0);gpio_set_level(IN2_PIN, 1);   \
    gpio_set_level(IN3_PIN, 1);gpio_set_level(IN4_PIN, 0);   \
}

#endif