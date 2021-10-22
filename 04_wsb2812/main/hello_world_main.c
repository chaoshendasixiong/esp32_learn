/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "rom/gpio.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define BLINK_GPIO 13 // 对应 nodemcu 的D7 通信
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<BLINK_GPIO)  // 配置GPIO_OUT位寄存器

static inline void delay_ns(uint32_t delta) {
    uint32_t cycleCount;
    uint32_t waitUntil;
    __asm__ __volatile__("rsr     %0, ccount":"=a" (waitUntil));
    waitUntil += delta;
    do {
        __asm__ __volatile__("rsr     %0, ccount":"=a" (cycleCount));
    } while(waitUntil > cycleCount);
}

#define DELAY400  delay_ns(40);
#define DELAY850  delay_ns(80);

#define SEND_0 {\
    gpio_set_level(BLINK_GPIO, 1);\
    DELAY400;\
    gpio_set_level(BLINK_GPIO, 0);\
    DELAY850;\
}
#define SEND_1 {\
    gpio_set_level(BLINK_GPIO, 1);\
    DELAY850;\
    gpio_set_level(BLINK_GPIO, 0);\
    DELAY400;\
}

#define RES {\
    gpio_set_level(BLINK_GPIO, 1);\
    ets_delay_us(50); \
    gpio_set_level(BLINK_GPIO, 0);\
}

#define RANDOM255 (esp_random()%255)
#define LED_NUM 1

void send_byte(unsigned char color) {
    int i;
    for(i = 0; i < 8; i++) {
        if(color & 0x80) {
            SEND_1; 
        } else {
            SEND_0;
        }
        color <<= 1;
    } 
}

void blink_task1(void *args) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    while(1){
        unsigned g=0,r=0,b=0;
        g = RANDOM255;
        r = RANDOM255;
        b = RANDOM255;
        send_byte(g);
        send_byte(r);
        send_byte(b);
        RES;
        printf("g=%d r=%d b=%d\n", g, r, b);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main() {
    uart_set_baudrate(0,115200);
    printf("Hello esp32 !\n");
    ESP_ERROR_CHECK(nvs_flash_init());
   
    blink_task1(NULL);
}
 