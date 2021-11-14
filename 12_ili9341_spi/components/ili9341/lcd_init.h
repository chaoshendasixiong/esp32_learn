#ifndef __h_ili9341_tft_h__
#define __h_ili9341_tft_h__

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi.h"
#include "rom/ets_sys.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 320

#else
#define LCD_W 320
#define LCD_H 240
#endif

#define u8  unsigned char
#define u16 unsigned int

#define MY_CLK_PIN   14 // D5
#define MY_MOSI_PIN  13 // D7
#define MY_RES_PIN   5 // D1
#define MY_DC_PIN    4 // D2
#define MY_BLK_PIN  16 //D0

void LCD_Init(void);//LCD初始化
void LCD_WR_DATA8(u8 dat) ;//写入一个字节
void LCD_WR_DATA(u16 dat) ;//写入两个字节
void LCD_WR_REG(u8 dat) ;//写入一个指令
void LCD_WR_ARRAY(uint8_t* pData, uint32_t len) ;
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数

#endif