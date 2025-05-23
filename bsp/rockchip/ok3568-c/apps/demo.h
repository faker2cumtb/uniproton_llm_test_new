#ifndef __DEMO_H__
#define __DEMO_H__
#include <prt_typedef.h>
#include <prt_buildef.h>
#include <rtdevice.h>

/******************* configure *******************/
/* timer demo */
#define DEMO_TIMER_DEV "timer0"

/* gpio demo */
#define DEMO_GPIO_INPUT_PIN "P3.A6"
#define DEMO_GPIO_OUTPUT_PIN "P3.A7"
#define DEMO_GPIO_IRQ_MODE PIN_IRQ_MODE_FALLING

/* uart demo */
#define DEMO_UART_DEV "uart3"
#define DEMO_UART_BAUD_RATE 115200
#define DEMO_UART_DATA_BITS DATA_BITS_8
#define DEMO_UART_STOP_BITS STOP_BITS_1
#define DEMO_UART_PARITY PARITY_NONE

/******************* functions *******************/
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void gpio_demo(void);
void uart_demo(void);
void timer_demo(void);
void smp_demo(void);

#endif
