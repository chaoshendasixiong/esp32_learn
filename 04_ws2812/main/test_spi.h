#ifndef __h__test_spi_h__
#define __h__test_spi_h__

#include "esp8266/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/spi.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define RED 0xff0000
#define GREEN 0x00ff00
#define BLUE 0x0000ff
#define BLACK 0x00
#define LED_NUM 8

#ifndef MIN
   #define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
   #define MAX(x,y) ((x)>(y)?(x):(y))
#endif

typedef struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RgbColor;

typedef struct HsvColor {
    uint16_t h;//[0~360]
    uint8_t s;//[0~100]
    uint8_t v;//[0~100]
} HsvColor;

static RgbColor hsv2rgb(HsvColor hsv) {
    RgbColor rgb;
    uint16_t hi = (hsv.h / 60) % 6;
    uint16_t F = 100 * hsv.h / 60 - 100 * hi;
    uint16_t P = hsv.v * (100 - hsv.s) / 100;
    uint16_t Q = hsv.v * (10000 - F * hsv.s) / 10000;
    uint16_t T = hsv.v * (10000 - hsv.s * (100 - F)) / 10000;

    switch (hi) {
        case 0: rgb.r = hsv.v;   rgb.g = T;     rgb.b = P;      break;
        case 1: rgb.r = Q;       rgb.g = hsv.v; rgb.b = P;      break;
        case 2: rgb.r = P;       rgb.g = hsv.v; rgb.b = T;      break;
        case 3: rgb.r = P;       rgb.g = Q;     rgb.b = hsv.v;  break;
        case 4: rgb.r = T;       rgb.g = P;     rgb.b = hsv.v;  break;
        case 5: rgb.r = hsv.v;   rgb.g = P;     rgb.b = Q;      break;
    }
    rgb.r = rgb.r * 255 / 100;
    rgb.g = rgb.g * 255 / 100;
    rgb.b = rgb.b * 255 / 100;
    return rgb;
}

static HsvColor rgb2hsv(RgbColor rgb) {
    HsvColor hsv;
    double hue, saturation, value;
    double m_max = MAX(rgb.r, MAX(rgb.g, rgb.b));
    double m_min = MIN(rgb.r, MIN(rgb.g, rgb.b));
    double m_delta = m_max - m_min;
    value = m_max / 255.0;
    if (m_delta == 0) {
        hue = 0;
        saturation = 0;
    } else {
        saturation = m_delta / m_max;
        if (rgb.r == m_max) {
            hue = (rgb.g - rgb.b) / m_delta;
        } else if (rgb.g == m_max) {
            hue = 2 + (rgb.b - rgb.r) / m_delta;
        } else {
            hue = 4 + (rgb.r - rgb.g) / m_delta;
        }
        hue = hue * 60;
        if (hue < 0) {
            hue = hue + 360;
        }
    }
    hsv.h = (int)(hue + 0.5);
    hsv.s = (int)(saturation * 100 + 0.5);
    hsv.v = (int)(value * 100 + 0.5);
    return hsv;
}

// 使用SPI的MOSI来输出高低电平信号驱动
// SPI先发送高位 
// esp32的pio13(hspi mosi)
#define CODE_0 0xC0 // 11000000
#define CODE_1 0xFC // 11111100
#define RANDOM255 (esp_random()%255)
static void IRAM_ATTR spi_event_callback(int event, void *arg) {}
static void init() {
    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Cancel hardware cs
    spi_config.interface.cs_en = 0;
    // MISO pin is used for DC
    spi_config.interface.miso_en = 0;
    // CPOL: 1, CPHA: 1
    spi_config.interface.cpol = 1;
    spi_config.interface.cpha = 1;
    // Set SPI to master mode
    // 8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_5MHz_DIV;
    spi_config.event_cb = spi_event_callback;
    spi_init(HSPI_HOST, &spi_config);
}

uint8_t buf[24*1024];
static inline void send_color(uint32_t *color, uint32_t size) {
    int i,j;
    int len = size * 3 * 8;
    memset(buf, 0x00, len);
    for(i = 0; i< size; i++) {
        uint8_t R = (color[i] >> 16);
        uint8_t G = (color[i] >>  8);
        uint8_t B = (color[i]      );
        printf("R=%d G=%d B=%d\n", R, G, B);
        for(j = 0; j < 8; j++) {
            if((G << j) & 0x80) {
                buf[i*24+j] = CODE_1; 
            } else {
                buf[i*24+j] = CODE_0;
            }
            if((R << j) & 0x80) {
                buf[i*24+j+8] = CODE_1;
            } else {
                buf[i*24+j+8] = CODE_0;
            }
            if((B << j) & 0x80) {
                buf[i*24+j+16] = CODE_1; 
            } else {
                buf[i*24+j+16] = CODE_0;
            }
        }
    }

    spi_trans_t trans;
    memset(&trans, 0x0, sizeof(trans));
    // mosi max send 64byte/cnt
    for(i = 0; i < len/64; i++) {
        trans.mosi = (uint32_t *)(buf+i*64);
        trans.bits.mosi = 64*8;// bit单位
        spi_trans(HSPI_HOST,&trans);
    }
    trans.mosi = (uint32_t *)(buf+len%64);
    trans.bits.mosi = (len%64)*8;// bit单位
    spi_trans(HSPI_HOST,&trans);
}

uint16_t h = 0;
void testhsv_breath1(uint32_t *color, int size) {
    uint8_t s = 100;
    uint8_t v = 100;
    h = (h+3)%360;
 
    HsvColor hsv = {.h = h,.s = s,.v = v};
    
    for (int i = 0; i < size; i++) {
        RgbColor rgb = hsv2rgb(hsv);
        color[i] = rgb.r<<16|rgb.g<<8|rgb.b;
        printf("r=%d g=%d b=%d\n",rgb.r, rgb.g, rgb.b);
    }
}
 
void testhsv_breath2(uint32_t *color, int size) {
    uint8_t s = 100;
    uint8_t v = 100;
    h = (h+3)%360;
 
    HsvColor hsv = {.h = h,.s = s,.v = v};
    
    for (int i = 0; i < size; i++) {
        hsv.h =( hsv.h + 360/size)%360;
        RgbColor rgb = hsv2rgb(hsv);
        color[i] = rgb.r<<16|rgb.g<<8|rgb.b;
        printf("r=%d g=%d b=%d\n",rgb.r, rgb.g, rgb.b);
    }
}

void blink_task(void *args) {
    init();
    uint32_t color[LED_NUM] = {0};

    while(1){ 
        send_color(color, sizeof(color)/sizeof(uint32_t));
        testhsv_breath1(color, sizeof(color)/sizeof(uint32_t));
        vTaskDelay(1000/portTICK_PERIOD_MS/30);
    }
}

#endif