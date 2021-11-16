#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "rom/gpio.h"
#include "driver/uart.h" 
#include "esp_event_loop.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "debugtool.h"
#define TAG "example"

static void initialize_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
static EventGroupHandle_t test_event_group;

void mydebug(uint8_t *buf)
{
    uint8_t cmd = atoi((const char *)buf);
    printf("cmd = %d!\n", cmd);

    switch (cmd)
    {
    case 1:
        xEventGroupSetBits(test_event_group, BIT0);
        break;
    case 2:
        xEventGroupSetBits(test_event_group, BIT1);
        break;

    default:
        break;
    }
}
static void test_task(void *args)
{
    ESP_LOGI(TAG, "test_task");
    EventBits_t bits;
#define BLINK_GPIO 2
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        bits = xEventGroupWaitBits(test_event_group, BIT0 | BIT1, true, false, portMAX_DELAY);
        ESP_LOGI(TAG, "bits = %d", bits);
        if (bits & BIT0)
        {
            gpio_set_level(BLINK_GPIO, 0);
        }
        if (bits & BIT1)
        {
            gpio_set_level(BLINK_GPIO, 1);
        }

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    initialize_nvs();
    myuart_init();
    test_event_group = xEventGroupCreate();
    xTaskCreate(&test_task, "test_task", 2048, NULL, 5, NULL);
}
