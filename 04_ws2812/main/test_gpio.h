#ifndef __h__test_gpio_h__
#define __h__test_gpio_h__
#include "rom/gpio.h"
#include "driver/gpio.h"
#include "esp_clk.h"

#define BLINK_GPIO 13 // 对应 nodemcu 的D7 通信
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<BLINK_GPIO)  // 配置GPIO_OUT位寄存器
static int cpu_freq;


static inline void delay_ns(uint32_t ns) {
    uint32_t tick = ns;
    //uint32_t tick = ns*cpu_freq/1000;
    //tick /= 2;
    uint32_t cur_tick;
    uint32_t delay_tick;
    __asm__ __volatile__("rsr     %0, ccount":"=a" (delay_tick)); // 会溢出 不可作为较长时间的延迟标准
    delay_tick += (tick -1);
    do {
        __asm__ __volatile__("rsr     %0, ccount":"=a" (cur_tick));
    } while(delay_tick > cur_tick);
}

static inline void gpio_set_level_fast(uint32_t pin, uint8_t val) {
    if (GPIO_IS_VALID_GPIO(pin)) {
        if (!RTC_GPIO_IS_VALID_GPIO(pin)) {
            if (val)
                GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1ul << pin);
            else
                GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1ul << pin);
        } else {
            if (val)
                REG_SET_BIT(RTC_GPIO_OUT, 1);
            else
                REG_CLR_BIT(RTC_GPIO_OUT, 1);
        }
    }
}

#define SET_LEVEL_h(pin) gpio_set_level_fast(pin, 1);
#define SET_LEVEL_L(pin) gpio_set_level_fast(pin, 0);

#define SEND_0 {\
    SET_LEVEL_h(BLINK_GPIO);\
    delay_ns(30);\
    SET_LEVEL_L(BLINK_GPIO);\
    delay_ns(80);\
}
#define SEND_1 {\
    SET_LEVEL_h(BLINK_GPIO);\
    delay_ns(90);\
    SET_LEVEL_L(BLINK_GPIO);\
    delay_ns(20);\
}

#define RES {\
    SET_LEVEL_L(BLINK_GPIO);\
    ets_delay_us(50); \
}

#define RANDOM255 (esp_random()%255)
#define LED_NUM 1

static inline void send_color(uint8_t green, uint8_t red, uint8_t blue) {
    taskENTER_CRITICAL();
    int i;
    for(i = 0; i < 8; i++) {
        if((green<<i) & 0x80) {
            SEND_1; 
        } else {
            SEND_0;
        }
    }
    for(i = 0; i < 8; i++) {
        if((red<<i) & 0x80) {
            SEND_1; 
        } else {
            SEND_0;
        }
    }
    for(i = 0; i < 8; i++) {
        if((blue<<i) & 0x80) {
            SEND_1; 
        } else {
            SEND_0;
        }
    } 
    taskEXIT_CRITICAL();
}

void blink_task(void *args) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(BLINK_GPIO, 0);
    cpu_freq = esp_clk_cpu_freq()/1000000;
    printf("cpu freq=%d\n", cpu_freq);
    unsigned g=0,r=0,b=0;
    int i;
    while(1){
        for(i = 0; i < LED_NUM; i++) { 
            g = RANDOM255;
            r = RANDOM255;
            b = RANDOM255;
            send_color(g,r,b);
        }
        //RES;
        vTaskDelay(1000/portTICK_PERIOD_MS);
        printf("g=%d r=%d b=%d\n", g, r, b);
    }
}


#endif