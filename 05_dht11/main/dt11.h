#ifndef __h_dt11_h__
#define __h_dt11_h__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"

static int gpio_index;
// 等待指定tick内电平变化
static int waitGpioLevel(int tick, int level) {
    int wait = 0;
    while(gpio_get_level(gpio_index) == level) {
        if(wait++ > tick) return -1;
        ets_delay_us(1);
    }
    return wait;
}
// 高40us 低80us 高80us
static inline int dht11_resp() {
    int ret;
    ret = waitGpioLevel(40, 1);
    if(ret < 0) return -1;
    ret = waitGpioLevel(80, 0);
    if(ret < 0) return -2;
    ret = waitGpioLevel(80, 1);
    if(ret < 0) return -3;
    return 0;
}
// 低电平50us 高电平28us -> 0
// 低电平50us 高电平70us -> 1
static inline int get_data() {
    int ret;
    ret = waitGpioLevel(50, 0);
    if(ret < 0) return -1;
    ret = waitGpioLevel(70, 1);
    if(ret < 0) return -2;
    if(ret > 28) return 1;
    else return 0;
}

void test_dht11(int io) {
    
    gpio_index = io;
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;  // 禁止中断  
    io_conf.mode = GPIO_MODE_OUTPUT;            // 选择输出模式
    io_conf.pin_bit_mask = (1ULL<<gpio_index); // 配置GPIO_OUT寄存器
    io_conf.pull_down_en = 0;                   // 禁止下拉
    io_conf.pull_up_en = 0;                     // 禁止上拉
    gpio_config(&io_conf); 
    gpio_set_level(gpio_index, 1); //初始高电平
    

    uint8_t data[5] = {0,0,0,0,0};
    float hum = 0;
    float temp = 0;
    int value = 0;
    
    while(1) {
        gpio_set_level(gpio_index, 1); //初始高电平
        gpio_set_direction(gpio_index, GPIO_MODE_OUTPUT);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        int i,j,ret;
        // 进制中断
        taskENTER_CRITICAL();
        // 发送启动信号
        gpio_set_level(gpio_index, 0);
        // 总线拉低必须大于18毫秒,保证DHT11能检测到起始信号
        ets_delay_us(20 *1000);
        gpio_set_level(gpio_index, 1);
        gpio_set_direction(gpio_index, GPIO_MODE_INPUT);
        // 等待dht11信号
        ret = dht11_resp();
        if(ret) {
            printf("no response %d!\n", ret);
            taskEXIT_CRITICAL();
            continue;
        }
        // 接受数据
        for (i = 0; i < 40; i++) {
            //data[i] = data[i] << 1;
            value = get_data();
            if(value < 0) printf("get date error %d\n", value);
            data[i/8] <<= 1;
            data[i/8] |= value;
            //data[i/8] |= (1 << (7-(i%8)));
        } 
        taskEXIT_CRITICAL();
        
        hum = data[0];
        temp = data[2] + (data[3] / 10.0);
        printf("temp = %d.%d\thum = %d \n", (int)temp, data[3], (int)hum);
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            printf("Checksum failed, invalid data received from sensor\n");
            continue;
        }
    }
}


#endif