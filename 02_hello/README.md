# hello esp32

使用[ssh工具](https://github.com/Eugeny/tabby)连接到 ubuntu 开始操作

```
cd ~/esp32/ESP8266_RTOS_SDK
mkdir esp32_learn
cd esp32_learn
cp -r ~/esp32/ESP8266_RTOS_SDK/examples/get-started/hello_world/ ./02_hello
cd 02_hello
make menuconfig
make
```

主要修改下 Serial flasher config 的几个配置

esp01s flash是 8Mbit ，esp12f 是 32Mbit

spi mode 具体要看 flash 芯片的型号 一般qio dio都行

默认串口打印波特率是74880 连按 esc 退出

最后能看到输出

```
 0x0 /home/zz/esp32/ESP8266_RTOS_SDK/esp32_learn/02_hello_esp32/build/bootloader/bootloader.bin 
 0x10000 /home/zz/esp32/ESP8266_RTOS_SDK/esp32_learn/02_hello_esp32/build/hello-world.bin 
 0x8000 /home/zz/esp32/ESP8266_RTOS_SDK/esp32_learn/02_hello_esp32/build/partitions_singleapp.bin
```

使用 ftp 工具下载固件之后 按照地址分别填上去 后面只用烧录0x1就行

烧录的时候 要 nodemcu 先进入刷机模式 按住Flash按键不动，再按一下（不长按）Reset（重启）按钮，闪一下蓝灯就可以松手了

烧录完成后 按下 RST 重启进入运行模式

```
esp_chip_info_t //芯片信息
vTaskDelay(1000 / portTICK_PERIOD_MS);//任务延时
```

FreeRTOS实时内核用tick count变量 来度量时间的 定时器中断的精度

```
#define portTICK_PERIOD_MS			( ( portTickType ) 1000 / configTICK_RATE_HZ )

#define configTICK_RATE_HZ			( ( portTickType ) CONFIG_FREERTOS_HZ )

#define CONFIG_FREERTOS_HZ 100 // sdkconfig.h
```

可以看到 tick 周期是100hz 即每秒100次  1个tick是10ms

vTaskDelay 100个tick即延时1s

