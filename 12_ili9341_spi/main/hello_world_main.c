#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"

#include "lcd_init.h"
#include "lcd.h" 
#include "pic.h"
 
void app_main() {
    //uart_set_baudrate(0,115200);
    u16 t = 0;
    LCD_Init();
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE); 
    while(1) {
        LCD_ShowString(0,40,(u8*)"LCD_W:",RED,WHITE,16,0);
		LCD_ShowIntNum(48,40,LCD_W,3,RED,WHITE,16);
		LCD_ShowString(80,40,(u8*)"LCD_H:",RED,WHITE,16,0);
		LCD_ShowIntNum(128,40,LCD_H,3,RED,WHITE,16);
		LCD_ShowString(80,40,(u8*)"LCD_H:",RED,WHITE,16,0);
		LCD_ShowString(0,70,(u8*)"Increaseing Nun:",RED,WHITE,16,0);
		LCD_ShowIntNum(128,70,t,4,RED,WHITE,16);
        t++; 
		 
        LCD_ShowPicture( 0, 110,48,64,gImage_111);
		 
        vTaskDelay(1000/portTICK_PERIOD_MS);
        printf("test!\n");
    }
  
}
