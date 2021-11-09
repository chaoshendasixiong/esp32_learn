#ifndef __h_debugtool_h__
#define __h_debugtool_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"
#include "driver/i2c.h"

#define UART_MAX_BUFFER (512)
#define UART_BUF_SIZE (512)
static QueueHandle_t s_uart0Queue;
static const char *TAG = "board_uart";
static void uartEventTask(void *pvParameters);
extern void mydebug(uint8_t *buf);
void myuart_init() {
    uart_config_t conf = {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK( uart_param_config(UART_NUM_0, &conf));
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, UART_BUF_SIZE*2, UART_BUF_SIZE*2, 100, &s_uart0Queue, 0));
    xTaskCreate(uartEventTask, "uartEventTask", 2048, NULL, 12, NULL);
}
static void uartEventTask(void *pvParameters) {
    uart_event_t event;
    uint8_t *buf = (uint8_t*)malloc(UART_MAX_BUFFER);
    for(;;) {
        if(xQueueReceive(s_uart0Queue, (void*)&event, (portTickType)portMAX_DELAY)) {
            memset(buf, 0x00, UART_MAX_BUFFER);
            switch (event.type) {
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(UART_NUM_0, buf, event.size, portMAX_DELAY);
                mydebug(buf);
                break;
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(UART_NUM_0);
                xQueueReset(s_uart0Queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(UART_NUM_0);
                xQueueReset(s_uart0Queue);
                break;
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(buf);
    buf = NULL;
    vTaskDelete(NULL);
}













#endif