#ifndef __DEMO_H__
#define __DEMO_H__
#include <prt_typedef.h>
#include <prt_buildef.h>
#include <rtdevice.h>

/******************* configure *******************/
/* timer demo */
#define DEMO_TIMER_DEV "timer6"

/* gpio demo */
#define DEMO_GPIO_INPUT_PIN "P0.C4"
#define DEMO_GPIO_OUTPUT_PIN "P0.C5"
#define DEMO_GPIO_IRQ_MODE PIN_IRQ_MODE_FALLING

/* uart demo */
#define DEMO_UART_DEV "uart0"
#define DEMO_UART_BAUD_RATE 115200
#define DEMO_UART_DATA_BITS DATA_BITS_8
#define DEMO_UART_STOP_BITS STOP_BITS_1
#define DEMO_UART_PARITY PARITY_NONE

/* spi xl2515 demo */
#define DEMO_SPI_XL2515_BUS "spi0"
#define DEMO_SPI_XL2515_DEV "spi0_0"
#define DEMO_SPI_XL2515_DATA_WIDTH 8
#define DEMO_SPI_XL2515_MODE (RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB)
#define DEMO_SPI_XL2515_BAUD 10000000
#define DEMO_SPI_XL2515_CS_PIN "P3.D3"

/* i2c ds1307 demo */
#define DEMO_I2C_DS1307_DEV "i2c0"

/* can demo */
#define DEMO_CAN_DEV "can1"
#define DEMO_CAN_MODE RT_CAN_MODE_NORMAL
#define DEMO_CAN_BAUD 1000000

/* mac demo */
#define DEMO_MAC_DEV "gmac0"

/******************* functions *******************/
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void gpio_demo(void);
void uart_demo(void);
void timer_demo(void);
void can_demo(void);
void spi_xl2515_demo(void);
void i2c_ds1307_demo(void);
void mac_demo(void);
void canopen_demo(void);
void smp_demo(void);
void ethercat_demo();
void pcie_its_demo();
void verify_demo();

#ifdef OS_SUPPORT_CANOPEN
void canopen_demo();
#endif

#ifdef OS_SUPPORT_MODBUS
int Modbus_Client_RTU_Demo();
int Modbus_Server_RTU_Demo();
int Modbus_Client_TCP_Demo();
int Modbus_Server_TCP_Demo();
#endif

#if defined(SECURITY_MBEDTLS)
void mbedtls_demo();
#endif

#if defined(OS_OPTION_PROXY)
int proxy_udp_demo();
#endif

#if defined(OS_OPTION_LOG)
void log_demo();
#endif

#endif
