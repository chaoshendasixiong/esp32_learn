#ifndef __h__test_spi_h__
#define __h__test_spi_h__


#include "esp8266/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/spi.h"

#define RANDOM255 (esp_random()%255)
#define BLINK_GPIO 13 // 对应 nodemcu 的D7 通信
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<BLINK_GPIO)  // 配置GPIO_OUT位寄存器

static void ws2812_spi_init() {
    uint8_t x = 0;
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
    spi_config.clk_div = SPI_8MHz_DIV;
    spi_config.event_cb = NULL;
    spi_init(HSPI_HOST, &spi_config);
}

//GRB format,MSB firsr.
//未提供按字节写数据函数，因为函数切换重启HSPI会影响时序
int WS2812BSend_24bit(uint8_t R, uint8_t G, uint8_t B) {
    uint32_t GRB = G << 16 | R << 8 | B;
    uint8_t data_buf[24];
    uint8_t *p_data = data_buf;

    //能用　等待优化!
    uint8_t mask = 0x80;
    uint8_t byte = G;

    while (mask) {
        if (byte & mask) {
          *p_data = 0xFC; /*11111100b;*/
        } else {
          *p_data = 0XC0; /*11000000b;*/
        }
        mask >>= 1;
        p_data++;
    }
    mask = 0x80;
    byte = R;
    while (mask) {
        if (byte & mask) {
          *p_data = 0xFC; /*11111100b;*/
        } else {
          *p_data = 0XC0; /*11000000b;*/
        }
        mask >>= 1;
        p_data++;
    }

    mask = 0x80;
    byte = B;

    while (mask) {
        if (byte & mask) {
          *p_data = 0xFC; /*11111100b;*/
        } else {
          *p_data = 0XC0; /*11000000b;*/
        }
        mask >>= 1;
        p_data++;
    }

    uint8_t *p_8_data;
    for (int i = 0; i < 6; i++) {
        p_8_data = (data_buf + (i * 4));
        uint8_t temp;
        for (int j = 0; j < 2; j++) {
          temp = p_8_data[j];
          p_8_data[j] = p_8_data[3 - j];
          p_8_data[3 - j] = temp;
        }
    }

    uint32_t *spi_buf = (uint32_t *)data_buf;
    spi_trans_t trans = {0};
    trans.mosi = spi_buf;
    trans.bits.mosi = 24 * 8;
    return spi_trans(HSPI_HOST,&trans);
}

void blink_task(void *args) {
    ws2812_spi_init();
    while(1){
        unsigned g=0,r=0,b=0;
        g = RANDOM255;
        r = RANDOM255;
        b = RANDOM255;
        WS2812BSend_24bit(g,r,b);
        printf("g=%d r=%d b=%d\n", g, r, b);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

#endif