#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define STB_PIN   13
#define CLK_PIN   12
#define DIO_PIN   14

#define TURN_ON 0x88
#define TURN_OFF 0x80
#define BRIGHTNESS 0x8F // 0x88~0x8F 8档脉冲宽度

#define READ_KEY 0x42 // 读键扫数据
// 写数据 2种方式
#define AUTO_ADDRESS 0x40 // 自动地址增加
#define Fix_ADDRESS 0x44 // 固定地址
#define BASE_ADDRESS 0xC0 //显示寄存器的起始地址 0~16

#define CS_START gpio_set_level(STB_PIN, 0); 
#define CS_STOP  gpio_set_level(STB_PIN, 1);

void sendCommand(uint8_t cmd);
// 0~9 A~F
uint8_t HEX_SEG[16]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
// LSB->MSB
static inline void writeData(uint8_t *pCmd, uint16_t len) {
    int i, j;
    gpio_set_direction(DIO_PIN, GPIO_MODE_OUTPUT);
    for(i = 0; i < len; i++) {
        for(j = 0; j < 8; j++) {
            gpio_set_level(CLK_PIN, 0);
            os_delay_us(1); 
            gpio_set_level(DIO_PIN, ( (pCmd[i] >> j) & 0x01 ));
            gpio_set_level(CLK_PIN, 1);
            os_delay_us(1);
        }
    } 
}
static inline void readData(uint8_t *inBuf, uint8_t iLen) {
 
    uint8_t tmp = 0; 
    int i, j;
    gpio_set_direction(DIO_PIN, GPIO_MODE_INPUT);
    for(i = 0; i < iLen; i++) {
        for(j = 0, tmp = 0; j < 8; j++) {
            gpio_set_level(CLK_PIN, 0);
            os_delay_us(1);
            gpio_set_level(CLK_PIN, 1);
            if(gpio_get_level(DIO_PIN)) {
                tmp |= (0x01<<j);
            }
            os_delay_us(1);
        }
        inBuf[i] = tmp;
        os_delay_us(2);
    }
}
// MSB - LSB
// 8个按键分别对应每个字节的第0位和第4位
// 0 0 0 1 (S5) 0 0 0 1 (S1)
// 0 0 0 1 (S6) 0 0 0 1 (S2)
// 0 0 0 1 (S7) 0 0 0 1 (S3)
// 0 0 0 1 (S8) 0 0 0 1 (S4)
void readKeys() {
    uint8_t recvBuf[4];
    uint8_t cmd = READ_KEY;
    memset(recvBuf, 0x00, 4);
    CS_START; 
    writeData(&cmd, 1);
    os_delay_us(3);
    readData(recvBuf, 4);
    CS_STOP;
    int i, j;
    printf("\n-----\n");
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            if(recvBuf[i] & (0x01 << (7-j) ) ) printf("1 ");
            else printf("0 ");
        }
        printf("\n");
    }
    printf("\n-----\n");
}


void sendCommand(uint8_t cmd) {
    gpio_set_direction(DIO_PIN, GPIO_MODE_OUTPUT);
    CS_START;
    writeData(&cmd, 1); 
    CS_STOP;
    os_delay_us(2);
}

void sendDatas(uint8_t address, uint8_t *data, int len) {
    CS_START; 
    writeData(&address, 1); 
    writeData(data, len); 
    CS_STOP;
    os_delay_us(2);
}

void setData(uint8_t index, uint8_t data) {
    sendCommand(Fix_ADDRESS);
    CS_START; 
    uint8_t address = BASE_ADDRESS+index;
    writeData(&address, 1); 
    writeData(&data, 1); 
    CS_STOP;
    os_delay_us(2);
}

void reset() {
    sendCommand(BRIGHTNESS);
    sendCommand(AUTO_ADDRESS); 
    uint8_t resetvalue[16];
    memset(resetvalue, 0x00, sizeof(resetvalue));
    sendDatas(BASE_ADDRESS, resetvalue, 16);
}


void test( ) { 
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_OUTPUT;            
    io_conf.pin_bit_mask = (1ULL<<STB_PIN)|(1ULL<<CLK_PIN);  
    io_conf.pull_down_en = 0;  
    io_conf.pull_up_en = 0;     
    gpio_config(&io_conf);  
    io_conf.pin_bit_mask = (1ULL<<DIO_PIN) ; 
    gpio_config(&io_conf); 
    vTaskDelay(500 / portTICK_RATE_MS);
    reset();
    sendCommand(BRIGHTNESS);
    sendCommand(AUTO_ADDRESS); 
    int i = 0;
    int numberIndex  = 0;
    int ledIndex = 1;
    uint8_t dataBuf[16];
    memset(dataBuf, 0x00, sizeof(dataBuf));
    while(1) { 
        memset(dataBuf, 0x00, sizeof(dataBuf));
        dataBuf[i%16] = HEX_SEG[numberIndex%16];
        dataBuf[(i+1)%16] = ledIndex%2;
        sendDatas(BASE_ADDRESS, dataBuf, 16);

        
        i+=2;
        numberIndex++;
        ledIndex++;

        // 处理按键程序
        readKeys();
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}
void app_main() {
    printf("Hello esp32!\n");
    vTaskDelay(1000 / portTICK_RATE_MS);
    test( );
}