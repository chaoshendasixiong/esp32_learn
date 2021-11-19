#include "stdio.h"
#include "rgbcw.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h" 
#include "esp_event.h"
#include "driver/ledc.h" 
#include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "sdkconfig.h"

// ===== [rgbcw pwm gpio] =====
#define MY_LED_C  4     // D2
#define MY_LED_W  2     // D4
#define MY_LED_R  14    // D5
#define MY_LED_G  12    // D6
#define MY_LED_B  13    // D7
#define CW_MODE 1
#define RGB_MODE 2

// ===== [rgbcw pwm config ledc] =====
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (4)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (2)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1
#define LEDC_HS_CH2_GPIO       (14)
#define LEDC_HS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_HS_CH3_GPIO       (12)
#define LEDC_HS_CH3_CHANNEL    LEDC_CHANNEL_3
#define LEDC_HS_CH4_GPIO       (13)
#define LEDC_HS_CH4_CHANNEL    LEDC_CHANNEL_4 
#define LEDC_TEST_CH_NUM       (5)
#define LEDC_TEST_DUTY         (8192)
#define LEDC_TEST_FADE_TIME    (1000)
#define ledc_channel_OBJ(chan, gpio)            \
    {                                           \
        .channel    = chan,                     \
        .duty       = 0,                        \
        .gpio_num   = gpio,                     \
        .speed_mode = LEDC_HS_MODE,             \
        .hpoint     = 0,                        \
        .timer_sel  = LEDC_HS_TIMER             \
    } 


// ===== [nvs user save cfg] =====
#define NVS_CONFIG_NAME "my_rgbcw"
#define NVS_TABLE_NAME "NVS_TABLE"

// ===== [nvs user save cfg] =====
#define MAX_TEMPERATURE 6500
#define MIN_TEMPERATURE 2700
#define MIDDLE_TEMPERATURE 4650
#define INIT_TEMPERATURE 4800
#define INIT_BRIGHTNESS 100
#define PERCENT_TEMPERATURE ((MAX_TEMPERATURE-MIN_TEMPERATURE)/100)
#define LED_GET_PERCENT(v, min, max) ((max - v)*100/(max-min))

//pwm duty is [0:8192] 
struct MY_PWM_RGBCW {
    uint8_t c;// duty percent [0:100]
    uint8_t w;// duty percent [0:100]
    uint8_t r;// duty percent [0:100]
    uint8_t g;// duty percent [0:100]
    uint8_t b;// duty percent [0:100]
};
// 色温 = 暖光时长/（暖光时长+冷光时长）
struct MY_LIGHT {
    uint16_t temperature; // [2700:6500]
    uint8_t brightness;   // [0:100]
    uint16_t  h;          // [0:360]
    uint8_t s;            // [0:100]
    uint8_t v;            // [0:100]
};

struct rgbcw_ctx {
    EventGroupHandle_t event_group;
    struct MY_LIGHT light;
};

#define DEFALUT_MY_LIGHT {  \
    .temperature =  INIT_TEMPERATURE,     \
    .brightness =  INIT_BRIGHTNESS,     \
    .h =  0,        \
    .s =  0,        \
    .v =  0,        \
}
 
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
static struct MY_LIGHT deault_cfg() {
    struct MY_LIGHT cfg = DEFALUT_MY_LIGHT;
    return cfg;
}

static void reset() {
    struct MY_LIGHT _default = deault_cfg(); 
}

static void ledc_init() {
    int ch;
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER            // timer index
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        ledc_channel_OBJ(LEDC_HS_CH0_CHANNEL, LEDC_HS_CH0_GPIO),
        ledc_channel_OBJ(LEDC_HS_CH1_CHANNEL, LEDC_HS_CH1_GPIO),
        ledc_channel_OBJ(LEDC_HS_CH2_CHANNEL, LEDC_HS_CH2_GPIO),
        ledc_channel_OBJ(LEDC_HS_CH3_CHANNEL, LEDC_HS_CH3_GPIO),
        ledc_channel_OBJ(LEDC_HS_CH4_CHANNEL, LEDC_HS_CH4_GPIO), 
    };

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
        ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
    }
}

static int load_user_cfg(struct MY_LIGHT *cfg) {
    nvs_handle out_handle;
    uint32_t length = sizeof(struct MY_LIGHT);
    if (nvs_open(NVS_CONFIG_NAME, NVS_READWRITE, &out_handle) != ESP_OK) {
        printf("nvs_open failed!\n");
        return -1;
    }
    if (nvs_get_blob(out_handle, NVS_TABLE_NAME, cfg, &length) != ESP_OK) {
        printf("nvs_get_blob failed!\n");
        return -1;
    }

    printf(" user brigtness : %d , temperature %d \n", cfg->brightness, cfg->temperature);
    nvs_close(out_handle);
    return 0;
} 

static struct MY_PWM_RGBCW light2pwm(struct MY_LIGHT light, int mode) {
    struct MY_PWM_RGBCW pwm; 
    
    if(mode == CW_MODE) {
        pwm.c = 100 - LED_GET_PERCENT(light.temperature, MIN_TEMPERATURE, MAX_TEMPERATURE);
        pwm.w = LED_GET_PERCENT(light.temperature, MIN_TEMPERATURE, MAX_TEMPERATURE);
    
        pwm.c = pwm.c*light.brightness/100;
        pwm.w = pwm.w*light.brightness/100;  
    }else {
        HsvColor hsv = {.h = light.h,.s = light.s, .v = light.v};
        RgbColor rgb = hsv2rgb(hsv);
        pwm.r = rgb.r;
        pwm.g = rgb.g;
        pwm.b = rgb.b;
    }
    return pwm;
} 

void rgbcw_init() {
    ledc_init();
    

}
 

void test() {
    rgbcw_init(); 
    int ch = 0;
    int mode = RGB_MODE;
    while (1) {  
        struct MY_LIGHT light = {
            .brightness = 100,
            .temperature = 4650,
            .h = 63,
            .s = 100,
            .v = 100
        };
        struct MY_PWM_RGBCW  rgbcw = light2pwm(light, mode);
        // light_driver_set_ctb(uint8_t color_temperature, uint8_t brightness);
        if(mode == CW_MODE) {
            ledc_set_fade_with_time(LEDC_HS_MODE, 0, rgbcw.c*8192/100, LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, 0, LEDC_FADE_NO_WAIT);
            ledc_set_fade_with_time(LEDC_HS_MODE, 1, rgbcw.w*8192/100, LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, 1, LEDC_FADE_NO_WAIT); 
        }else {
            ledc_set_fade_with_time(LEDC_HS_MODE, 2, rgbcw.r*8192/255 , LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, 2, LEDC_FADE_NO_WAIT);
            ledc_set_fade_with_time(LEDC_HS_MODE, 3, rgbcw.g*8192/255 , LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, 3, LEDC_FADE_NO_WAIT);
            ledc_set_fade_with_time(LEDC_HS_MODE, 4, rgbcw.b*8192/255 , LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, 4, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(112000 / portTICK_PERIOD_MS);
          /*
        printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < 1; ch++) {
            ledc_set_fade_with_time(LEDC_HS_MODE,
                    ch, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, ch, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

        printf("2. LEDC fade down to duty = 0\n");
        for (ch = 0; ch < 1; ch++) {
            ledc_set_fade_with_time(LEDC_HS_MODE, ch, 0, LEDC_TEST_FADE_TIME);
            ledc_fade_start(LEDC_HS_MODE, ch, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);
       
        

        printf("3. LEDC set duty = %d without fade\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_TEST_DUTY);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("4. LEDC set duty = 0 without fade\n");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(100000 / portTICK_PERIOD_MS);
        */
    }
}
