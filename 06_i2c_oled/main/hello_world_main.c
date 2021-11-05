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
#include "driver/uart.h"
#include "driver/i2c.h"
#include "debugtool.h"
#include "ssd1306.h"
#include "font.h"

#define SCL_PIN 5
#define SDA_PIN 4
   
ssd1306_t *dev ;
ssd1306_t ssd_dev1 ;

void drawHLine(ssd1306_t *dev, uint8_t x, uint8_t y, int width) {
    int i;
    for(i = 0; i < width; i++) {
        drawPixel(dev, x+i, y);
    }
}
//x1 y1  |---|
//       |   |
//       |---| x2 y2
void drawChar6X8(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t ch) {
	uint8_t fontindex = ch - ' ';// 可显示字符从空格开始
	int buffIndex = x+(y/8)*128;
    int i = 0;
    for(i = 0; i < 6; i++) {
        dev->backbuffer[buffIndex+i] |= (D6X8[fontindex*6+i] << (y%8) );
    }
    buffIndex = x+(y/8 + 1)*128;
    for(i = 0; i < 6; i++) {
        dev->backbuffer[buffIndex+i] |= (D6X8[fontindex*6+i] >> (8 - y%8) );
    } 
}

void drawChar8X16(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t ch) {
	uint8_t fontindex = ch - ' ';// 可显示字符从空格开始
	int buffIndex = x+(y/8)*128;
    int i = 0; 
    for(i = 0; i < 8; i++) {
        dev->backbuffer[buffIndex+i] |= (D8X16[fontindex*16+i] << (y%8) );
    }
    buffIndex = x+(y/8 + 1)*128;
    for(i = 0; i < 8; i++) {
        dev->backbuffer[buffIndex+i] |= (D8X16[fontindex*16+i] >> (8 - y%8) );
        dev->backbuffer[buffIndex+i] |= (D8X16[fontindex*16+i+8] << (y%8) );
    }
    buffIndex = x+(y/8 + 2)*128;
    for(i = 0; i < 8; i++) {
        dev->backbuffer[buffIndex+i] |= (D8X16[fontindex*16+i+8] >> (8 - y%8) );
    }
}

void drawStr(ssd1306_t *dev, uint8_t x, uint8_t y, const char*str, int len) {
    int i;
    for(i = 0; i < len; i++) {
        drawChar6X8(dev, x+i*6, y, str[i]);
    }
}

void mydebug(uint8_t *buf) {
    uint8_t cmd = atoi((const char*)buf);
    printf("cmd = %d!\n", cmd);
    switch (cmd) {
    case 1: clearScreen(dev); break;
    case 2:
    {
        int i,j;
        for(i = 0; i < 8; i++) {
            for(j = 0; j < 8; j++) {
                drawChar6X8(dev, i*16, j*8, j+'A');
            }
        }
        sendBuffer(dev);
    }
    break; 
    case 3: scrollV(dev, 1); break;
    case 4: scrollV(dev, 2); break;
    case 5: scrollV(dev, 4); break;
    case 6: scrollH(dev, 2, 7, SPEED_2); break;
    case 7: sendBuffer(dev); break;
    default:
        break;
    }
}

void ssd1306_run(void *args) {
    ssd1306_t *dev = (ssd1306_t *)args;
    while(1) {
        updateScreen(dev);
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void app_main() {
    myuart_init();
    printf("Hello esp32!\n");
    vTaskDelay(1000 / portTICK_RATE_MS);
    dev = &ssd_dev1;
    ssd1306_init(SDA_PIN, SCL_PIN, &ssd_dev1);
    xTaskCreate(ssd1306_run, "ssd1306_run", 2048, dev, 10, NULL);
}
