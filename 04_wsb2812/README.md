# freertos 驱动 wsb2812 

[api文档](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-reference/index.html)

硬件：一个12位的 WSB2812 5050 RGB Led 全彩

* DI	控制输入
* 5v    电源
* GND 接地
* DO 控制输出 用于级联功能

引脚介绍如上 只需要一个单片机引脚就能控制

复制03_led -> 04_wsb2812 我们将通过2种方式来驱动led工作

| T0H  | 0  高电平 | 0.4us  |
| ---- | --------- | ------ |
| T0L  | 0 低电平  | 0.85us |
| T1H  | 1 高电平  | 0.85us |
| T1L  | 1 低电平  | 0.4us  |
| RES  | 帧 低电平 | 50us   |

在02hello中可以知道freertos的delay是tick单位 默认是10ms级别 

ets_delay_us()（在 rom/ets_sys.h 中定义）是微妙级别

所以要用别的方法实现纳秒延迟

不断计算时钟tick或者使用nop()实现忙等延迟

注意暂停中断和上下文切换带来的影响(portDISABLE_INTERRUPTS portENABLE_INTERRUPTS来禁止)

```
// esp8266 有个叫 ccount 的32位寄存器 不断计算时钟tick
// CPU 默认以 80MHz 的速度运行 （160Mhz需要配置）= 80*100w个clock cycle
// 1微妙80个tick 1个tick是12.5ns
static inline void NOP() { __asm__ __volatile__ ("nop"); }// NOP指令在大多数微处理器中占用一个周期
static inline uint32_t TICK_COUNT(void) {// 会溢出 不可作为较长时间的延迟标准
    uint32_t ticks;
    __asm__ __volatile__ ("rsr %0, ccount\n" : "=a"(ticks):: "memory");
    return ticks;
}

static uint32_t test_us_delay(uint32_t us) {
    uint32_t start, end;
    start = TICK_COUNT();
    os_delay_us(us);
    end = TICK_COUNT();
    printf("delay %dus tick=%d\n", us, start-end);
    return start-end;
}

static inline void delay_ns(uint32_t tick) {
    uint32_t cur_tick;
    uint32_t delay_tick;
    __asm__ __volatile__("rsr     %0, ccount":"=a" (delay_tick));
    delay_tick += tick;
    do {
        __asm__ __volatile__("rsr     %0, ccount":"=a" (cur_tick));
    } while(delay_tick > cur_tick);
}
```

## gpio 

根据上面的表格可以知道传输1个bit（0或者1）需要1.25us 

1个tick是12.5ns

## spi

















