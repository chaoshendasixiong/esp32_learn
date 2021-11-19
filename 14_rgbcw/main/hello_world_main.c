#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"

#include "rgbcw.h"

void app_main()
{
    //uart_set_baudrate(0,115200);

    while (1)
    { 
        test();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("test!\n");
    }
}
