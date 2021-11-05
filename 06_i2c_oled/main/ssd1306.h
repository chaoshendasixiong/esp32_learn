#ifndef __h_ssd1306_h__
#define __h_ssd1306_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/i2c.h"


#define SSD1306_w 128
#define SSD1306_h 64
#define SSD1306_I2C_ADDR_0    (0x78)
#define SSD1306_I2C_ADDR_1    (0x7A)
#define SSD1306_CTL_CMD     0x00
#define SSD1306_CTL_DATA    0x40
#define NUM_COLUMNS  128 
#define NUM_PAGES  8 
#define NUM_ROWS_PER_PAGE = 8;
#define DEFAULT_FONT_SIZE 8

typedef struct {
    i2c_port_t i2c_port;     //!< i2c port (bus) number
    uint8_t i2c_addr;        //!< ssd1306 device address
    uint8_t width;           //!< Screen width, currently supported 128px, 96px
    uint8_t height;          //!< Screen height, currently supported 16px, 32px, 64px
    uint8_t pin;
    uint8_t backbuffer[NUM_COLUMNS*NUM_PAGES];
    uint8_t frontbuffer[NUM_COLUMNS*NUM_PAGES];
    uint8_t isUpdate;
} ssd1306_t;


void ssd1306_init(uint32_t sda, uint32_t scl, ssd1306_t *dev);
static int ssd1306_command(const ssd1306_t *dev, uint8_t command);
static int ssd1306_data(const ssd1306_t *dev, uint8_t data);
static int i2c_send(uint8_t address, uint8_t i2c_port, uint8_t ctl_type, uint8_t data);

void ssd1306_setPos(const ssd1306_t *dev, uint8_t x, uint8_t y);
void clearScreen(ssd1306_t *dev);
void updateScreen(ssd1306_t *dev);
void sendBuffer(ssd1306_t *dev);
void drawPixel(ssd1306_t *dev, uint8_t x, uint8_t y) ;

//___________________________________________________________
//| start | addr + w + ack | crl + ack | data + ack  | stop |
//--------|----------------|-----------|-------------|------|
static int i2c_send(uint8_t address, uint8_t i2c_port, uint8_t ctl_type, uint8_t data) {
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, ctl_type, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
static int ssd1306_data(const ssd1306_t *dev, uint8_t data) {
    return i2c_send(dev->i2c_addr, dev->i2c_port, SSD1306_CTL_DATA, data);
}

static int ssd1306_command(const ssd1306_t *dev, uint8_t data) {
    return i2c_send(dev->i2c_addr, dev->i2c_port, SSD1306_CTL_CMD, data);
}

void ssd1306_init(uint32_t sda, uint32_t scl, ssd1306_t *dev) {
    int i2c_master_port = I2C_NUM_0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .sda_pullup_en = 1,
        .scl_io_num = scl,
        .scl_pullup_en = 1,
        .clk_stretch_tick = 300
    };
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    dev->i2c_port = i2c_master_port;
    dev->i2c_addr = SSD1306_I2C_ADDR_0;
    dev->width = SSD1306_w;
    dev->height = SSD1306_h;
    if(dev->height == 64) dev->pin = 0x12;
    else dev->pin = 0x02;
    memset(dev->backbuffer, 0x00, sizeof(dev->backbuffer));
    memset(dev->frontbuffer, 0x00, sizeof(dev->frontbuffer));
    dev->isUpdate = 0;
    uint8_t init_commands[] = {
        0xAE,
        0x20, 0x02, // 设置页寻址模式
        0x00, 0x10, // 设置列起始地址0
        0xB0, // 起始页0
        0x40, // 设置起始行0
        0xA8, 0x3F,// 设置复用率 63
        0xD3, 0x00,// 显示偏移0
        0x81, 0x7F,// 设置对比度
        0x2E,
        0xA1, // 列翻转 0->128
        0xC8, // com扫描方向
        0xD5, 0x80, //时钟1 震荡1000
        0xD9, 0xF1, //设置充电周期
        0xDA, 0x12, //com引脚12
        0xDB, 0x20, //com反压
        0xA4, // 设置RAM内容显示
        0xA6, // bit位正常显示
        0x8D, 0x14, // 设置电荷泵
        0xAF
    };    
    int i;
    for(i = 0; i < sizeof(init_commands)/sizeof(uint8_t); i++) {
        ssd1306_command(dev, init_commands[i]);
    }
    clearScreen(dev);
}
// x=[0~127] y=[0~63]
void ssd1306_setPos(const ssd1306_t *dev, uint8_t x, uint8_t y) {
    ssd1306_command(dev, 0xB0+y/8);
    ssd1306_command(dev, 0x0F & x);
    ssd1306_command(dev, 0x10 | (( 0xF0 & x) >> 4));
}
// x=[0~127] y=[0~63]
// LSB在上 MSB在下
void drawPixel(ssd1306_t *dev, uint8_t x, uint8_t y) {
    int index = x+(y/8)*128;
    dev->backbuffer[index] |= 0x1<<(y%8);
}

void sendBuffer(ssd1306_t *dev) {
    dev->isUpdate = 1;
}

void clearScreen(ssd1306_t *dev) {
    int i,j;
    for(i = 0; i < 8; i++) {
        ssd1306_command(dev, 0xB0+i);
        for(j = 0; j < 128; j++) {
            ssd1306_command(dev, 0x0F & j);
            ssd1306_command(dev, 0x10 | (( 0xF0 & j) >> 4));
            ssd1306_data(dev, 0);
        }
    }
    dev->isUpdate = 0;
    memset(dev->backbuffer, 0x00, sizeof(dev->backbuffer));
    memset(dev->frontbuffer, 0x00, sizeof(dev->frontbuffer));
}

void updateScreen(ssd1306_t *dev) {
    int i;
    uint8_t *back = &dev->backbuffer[0];
    uint8_t *front = &dev->frontbuffer[0];
    uint8_t x = 0, y = 0;
    if(!dev->isUpdate) return ;
    dev->isUpdate = 0;
    for(i = 0; i < sizeof(dev->backbuffer)/sizeof(uint8_t); i++) {
        if(front[i] != back[i]) {
            front[i] = back[i];
            x = i%128;
            y = i/128; 
            ssd1306_command(dev, 0xB0 + y);
            ssd1306_command(dev, 0x0F & x);
            ssd1306_command(dev, 0x10 | (( 0xF0 & x) >> 4));
            ssd1306_data(dev, front[i]);
        }
    }
}

void showScreen(ssd1306_t *dev) {
    ssd1306_command(dev, 0x8D);
    ssd1306_command(dev, 0x14);
    ssd1306_command(dev, 0xAF);
}
void hideScreen(ssd1306_t *dev) {
    ssd1306_command(dev, 0x8D);
    ssd1306_command(dev, 0x10);
    ssd1306_command(dev, 0xAE);
}
// between last scroll and next scroll need to wait frames
// 000b-5frames 001b-64frames 010b-128frames 011b-256 frames
// 100b-3frames 101b-4frames 110b-25frames 111b-2frames
enum SCROLL_SPEED {
    SPEED_5,
    SPEED_64,
    SPEED_128,
    SPEED_256,
    SPEED_3,
    SPEED_4,
    SPEED_25,
    SPEED_2
};
// pageTop 0~7
// pageBottom 0~7
// SPEED_5 is enough 
void scrollH(ssd1306_t *dev, uint8_t pageTop, uint8_t pageBottom, uint8_t frame) {
    ssd1306_command(dev, 0x2E);
    ssd1306_command(dev, 0x26);
    ssd1306_command(dev, 0x00); // dummy 00
    ssd1306_command(dev, pageTop); // page start
    ssd1306_command(dev, frame); // delta frame
    ssd1306_command(dev, pageBottom); // page end
    ssd1306_command(dev, 0x00); // dummy 00
    ssd1306_command(dev, 0xFF); // dummy FF

    ssd1306_command(dev, 0x2F);     
}
// speed 1~64
// page start and page end set 0x00 
void scrollV(ssd1306_t *dev, uint8_t speed) {
    ssd1306_command(dev, 0x2E);
    ssd1306_command(dev, 0x2A);
    ssd1306_command(dev, 0x00); // dummy 00
    ssd1306_command(dev, 0x00); // page start
    ssd1306_command(dev, 0x00); // delta frame
    ssd1306_command(dev, 0x00); // page end
    ssd1306_command(dev, speed); // offset

    ssd1306_command(dev, 0xA3); 
    ssd1306_command(dev, 0);      
    ssd1306_command(dev, dev->height);

    ssd1306_command(dev, 0x2F);     
}

#endif

