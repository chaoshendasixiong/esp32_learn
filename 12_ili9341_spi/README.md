# SPI ILI9341

esp8266-12F SMD-22封装 一共22个引脚

| 序号 | pin    | 说明                          |
| ---- | ------ | ----------------------------- |
| 1    | RST    | 复位                          |
| 2    | ADC    | 单通道ADC采样                 |
| 3    | EN     | 使能                          |
| 4    | GPIO16 | gpio16                        |
| 5    | GPIO14 | GPIO14 HSPI_CLK               |
| 6    | GPIO12 | GPIO12 HSPI_MISO              |
| 7    | GPIO13 | GPIO13 HSPI_MOSI uart_CTS     |
| 8    | VCC    | 3.3v 供电                     |
| 9    | CS0    | 片选                          |
| 10   | MISO   |                               |
| 11   | GPIO9  | GPIO9                         |
| 12   | GPIO10 |                               |
| 13   | MOSI   |                               |
| 14   | SCLK   |                               |
| 15   | GND    |                               |
| 16   | GPIO15 | GPIO15 MTDO HSPICS  UART0_RTS |
| 17   | GPIO2  | GPIO2 UART1_TXD               |
| 18   | GPIO0  | GPIO0                         |
| 19   | GPIO4  | GPIO4                         |
| 20   | GPIO5  | GPIO5                         |
| 21   | RXD    | GPIO3 UART0_RXD               |
| 22   | TXD    | GPIO1 UART0_TXD               |

剩17个gpio引脚 其中9-14 （gpio6-gpio11）不可用  连得是flash闪存

剩下的gpio0 电平不能改 (高电平工作模式 低电平下载模式)

gpio15 初始要低电平  0~15配上拉电阻 16配下拉电阻

GPIO16引脚只能用作GPIO读/写

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

2个硬件uart gpio1和gpio3 以及 gpio2和gpio8(连闪存)所以gpio2只能用来向外发送串行数据

IIC gpio软件模拟

SPI 

GPIO14 — CLK
GPIO12 — MISO
GPIO13 — MOSI
GPIO 15 — CS(SS)



SPI  bit位顺序 0x1F  0x00011111

## 引脚定义

| 定义 | 说明                                   |      |
| ---- | -------------------------------------- | ---- |
| GND  | 接地                                   |      |
| VCC  | 3.3v                                   |      |
| CLK  | 时钟线                                 |      |
| MOSI | 主机输出                               |      |
| RES  | 使能显示屏驱动芯片 低电平使能 复位作用 |      |
| DC   | 数据/指令选择                          |      |
| BLK  | 背光开关 默认打开 低电平关闭           |      |
| MISO | 主机输入 无用                          |      |
|      |                                        |      |

买了一个老款的TFT屏 不带触摸

相比较传统SPI协议， 这个屏幕不需要MOSI， 也不带片选线

DC来使能主机输出的是数据还是命令

240x320 rgb输出 一个像素点18bit GRAM=240x320x18/8 = 172800字节

720路源极 320栅极驱动 VCOM信号

RGB 5,6,5位 方向是MSB->LSB 第0位和第12位是无效位 所以rgb占了18bit

 

LI9341控制器内部设计了一个“控制寄存器”，用户只需要按照要求发送相应的控制指令，“控制寄存器”收到控制指令后，通过“地址计数器”在对应的像素位置写入像素数据，最终通过LCD源极驱动LCD刷新显示

## 指令列表

 













