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
#include "driver/gpio.h"

#include "dt11.h"
#define DHT11_IO 4

void app_main()
{
    uart_set_baudrate(0,115200);
    printf("Hello esp32 !\n");
    ESP_ERROR_CHECK(nvs_flash_init());
    test_dht11(DHT11_IO);
}
