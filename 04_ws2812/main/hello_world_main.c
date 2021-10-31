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

#include "driver/uart.h"

//#include "test_gpio.h"
#include "test_spi.h"

void app_main() {
    uart_set_baudrate(0,115200);
    ESP_ERROR_CHECK(nvs_flash_init());
    vTaskDelay(1000/portTICK_PERIOD_MS);
    printf("Hello esp3 --- !\n");  
    blink_task(NULL);
    //test();
}
 