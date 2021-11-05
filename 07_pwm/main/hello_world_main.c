/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/pwm.h"
#include "esp_log.h"
 

#define PWM_0_OUT_IO_NUM   12 

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD    (1000)
static const char *TAG = "pwm_example";
// pwm pin number
const uint32_t pin_num[4] = {
    PWM_0_OUT_IO_NUM, 
};
// duties table, real_duty = duties[x]/PERIOD
uint32_t duties = 500  ;
// phase table, delay = (phase[x]/360)*PERIOD
float phase[4] = {
    0, 0, 90.0, -90.0,
};
void app_main() {
    printf("Hello esp32!\n");
    vTaskDelay(1000 / portTICK_RATE_MS);
    // 设置IO口的PWM参数 周期1000us 占空比 500 
    pwm_init(PWM_PERIOD, &duties, 1, pin_num);
    pwm_set_phases(phase);
    pwm_start(); 
    uint16_t i;
    while (1) { 
        for (i = 0; i < duties; i++) {
            pwm_set_duty(0, i); 
            pwm_start(); 
            vTaskDelay(10 / portTICK_RATE_MS);
        }
        for (i = duties; i > 0; i--) {
            pwm_set_duty(0, i); 
            pwm_start();  
            vTaskDelay(10 / portTICK_RATE_MS);
        } 
        //vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
