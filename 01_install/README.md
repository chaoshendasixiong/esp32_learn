# 安装开发环境

1 安装 ubuntu 系统

2 安装依赖包

```
sudo apt-get install gcc git wget make libncurses-dev flex bison gperf python python-serial
```

3 下载工具链和 SDK

```
wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
tar -zxf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git
```

> 注意克隆要完整 否则编译会出现各种问题

4 设置环境变量

```
vim ~/.bashrc
export PATH=$HOME/esp32/xtensa-lx106-elf/bin:$PATH
export IDF_PATH=$HOME/esp32/ESP8266_RTOS_SDK
```

5 软件和硬件

软件包括烧录工具（flash_download_tools_v3.6.8），串口调试工具（sscom5.13.1），驱动（CH341SER）

硬件我自己买的是 esp8266-12F wifi 模块 NodeMCU CH340，其它包含 usb 数据线，杜邦线

esp01s 烧录参数 dout 1MB =8mbit

esp12f QIO  4MB=32Mbit

基于乐鑫方案的ESP8266选择QIO,ESP8285选择DOUT

Flash 常用的工作模式有 4 种：DOUT/DIO/QOUT/QIO
\* DOUT:地址为 1 线模式输入，数据为 2 线模式输出
\* DIO：地址为 2 线模式输入，数据为 2 线模式输出
\* QOUT：地址为 1 线模式输入，数据为 4 线模式输出
\* QIO：地址为 4 线模式输入，数据为 4 线模式输出

用户如果需要使用 QIO 模式，则需要在选择 Flash 时确认该 Flash 是否支持 QIO 模式
