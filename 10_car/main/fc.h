#ifndef __h_fc_control_h__
#define __h_fc_control_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define FC_LATCH_PIN  12
#define FC_CLOCK_PIN  14
#define FC_DATA_PIN   13

// A B Select Start up down left right
//         CLK  LATC  DATA    
// ________________________________
//  |_  0    X    X    X    0   _|    DB9 
//    |_   0    X    0    X   _| 
//      |____________________| 
//             GND       VCC  

void sendCmd(uint8_t cmd) {
    uint8_t bStr[9];
    int i;
    printf("cmd= %d\n",cmd);
    for(i = 0; i < 8; i++) {
        if((cmd >> i) & 0x1) {
            bStr[i] = '1';
        }else {
            bStr[i] = '0';
        }
    }
    bStr[i] = '\0';
    printf("cmd= %s\n",bStr);
}

void initFc() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_OUTPUT;            
    io_conf.pin_bit_mask = (1ULL<<FC_LATCH_PIN)|(1ULL<<FC_CLOCK_PIN);  
    io_conf.pull_down_en = 0;  
    io_conf.pull_up_en = 0;     
    gpio_config(&io_conf); 
    io_conf.mode = GPIO_MODE_INPUT; 
    io_conf.pin_bit_mask = (1ULL<<FC_DATA_PIN);
    gpio_config(&io_conf);  
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void test_fc( ) { 
    
    int i;
    uint8_t cmd = 0;
    while(1) {
        // latch start
        gpio_set_level(FC_LATCH_PIN, 1);
        os_delay_us(10);
        gpio_set_level(FC_LATCH_PIN, 0);
        os_delay_us(10);
        if(gpio_get_level(FC_DATA_PIN)) {
            cmd |= 0x1;
        }
        // clk to start
        for(i = 1; i < 8; i++) {
            gpio_set_level(FC_CLOCK_PIN, 1);
            os_delay_us(10);
            gpio_set_level(FC_CLOCK_PIN, 0);
            os_delay_us(10);
            if(gpio_get_level(FC_DATA_PIN)) {
                cmd |= (0x1 << i);
            }
        }
        // clk to end
        gpio_set_level(FC_CLOCK_PIN, 1);
        os_delay_us(10);
        gpio_set_level(FC_CLOCK_PIN, 0);
        os_delay_us(10);
        sendCmd(cmd);
        cmd = 0;
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}


#endif