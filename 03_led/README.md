# esp8266 板载led
复制02_hello 修改为03_led

使用gpio的方式，通过设置高低电平来控制板载的 led 闪烁

gpio的控制有2种方式

旧的在rom/gpio.h 新的在driver/gpio.h

功能的执行同样有2种方式

简单的就用while循环

RTOS提供的任务管理接口 xTaskCreate

```
portBASE_TYPE xTaskCreate(
	pdTASK_CODE pvTaskCode,				//任务函数指针
	const portCHAR * const pcName,		//任务函数的别名
	unsigned portSHORT usStackDepth,	//任务堆栈深度
	void *pvParameters,					//任务参数
	unsigned portBASE_TYPE uxPriority,	//任务优先级
	xTaskHandle *pvCreatedTask			//任务句柄
);
void vTaskDelete( xTaskHandle pxTask )	//任务删除
```

