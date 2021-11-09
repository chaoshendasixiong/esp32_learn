#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "car.h"
#include "debugtool.h"

void mydebug(uint8_t *buf) {
    uint8_t cmd = atoi((const char*)buf);
    printf("cmd = %d!\n", cmd);
    
    switch (cmd) {
    case 1:  carReset();  break; 
    case 2: FORWARD; break; 
    case 3: BACK; break; 
    case 4: LEFT; break; 
    case 5: RIGHT; break; 
    default:
        break;
    }
}

void test( ) { 
    myuart_init();
    initL298N();
}
void app_main() {
    printf("Hello esp32!\n");
    vTaskDelay(1000 / portTICK_RATE_MS);
    test( );
}