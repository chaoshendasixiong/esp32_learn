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
#include "mybutton.h"
 
#define BUTTON_GPIO 4
#define BLINK_GPIO 2
void blink_task1(void *args) {
    gpio_pad_select_gpio(BLINK_GPIO);//选择一个GPIO
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);//把这个GPIO作为输出
    while (1) {
        printf(" BLINK_GPIO =%d GPIO_NUM_2=%d\n", BLINK_GPIO, GPIO_NUM_2 );
        gpio_set_level(BLINK_GPIO, 1);//把这个GPIO输出低电平 
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(BLINK_GPIO, 0);//把这个GPIO输出低电平 
    }
}

#define GPIO_OUTPUT_PIN_SEL  (1ULL<<BLINK_GPIO)  // 配置GPIO_OUT位寄存器
void blink_task2(void *args) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;  // 禁止中断  
    io_conf.mode = GPIO_MODE_OUTPUT;            // 选择输出模式
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // 配置GPIO_OUT寄存器
    io_conf.pull_down_en = 0;                   // 禁止下拉
    io_conf.pull_up_en = 0;                     // 禁止上拉
    gpio_config(&io_conf);                      // 配置使能
    
    while (1) {
        printf(" BLINK_GPIO =%d GPIO_NUM_2=%d\n", BLINK_GPIO, GPIO_NUM_2 );
        gpio_set_level(BLINK_GPIO, 1);//把这个GPIO输出低电平 
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(BLINK_GPIO, 0);//把这个GPIO输出低电平 
    }
}

void blink_button_task(void *args) {
    gpio_pad_select_gpio(BLINK_GPIO);//选择一个GPIO
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);//把这个GPIO作为输出
    while (1) {
        gpio_set_level(BLINK_GPIO, myButton_state(BUTTON_GPIO));  
        vTaskDelay(10 / portTICK_RATE_MS); 
    }
}

void app_main()
{
    printf("Hello esp32 !\n");
    ESP_ERROR_CHECK(nvs_flash_init());
    myButton_init(BUTTON_GPIO);
    //blink_task1(NULL);
    //blink_task2(NULL);
    //xTaskCreate(&blink_task1, "blink_task1", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(&blink_button_task, "blink_task2", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
