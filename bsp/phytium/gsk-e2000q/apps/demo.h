#ifndef __DEMO_H__
#define __DEMO_H__
#include <prt_typedef.h>
#include <prt_buildef.h>
#include <rtdevice.h>

/******************* configure *******************/

/* demo timer */
#define DEMO_TIMER_DEV "timer0"

/* gpio demo */
#define DEMO_GPIO_INPUT_PIN "P2.8"
#define DEMO_GPIO_OUTPUT_PIN "P5.10"
#define DEMO_GPIO_IRQ_MODE PIN_IRQ_MODE_FALLING //e2000q不支持双边沿触发

/* uart demo */
#define DEMO_UART_DEV "mio_uart0"
#define DEMO_UART_BAUD_RATE 115200
#define DEMO_UART_DATA_BITS DATA_BITS_8
#define DEMO_UART_STOP_BITS STOP_BITS_1
#define DEMO_UART_PARITY PARITY_NONE

/* spi loopback demo */
#define DEMO_SPI_LOOPBACK_BUS "spi0"
#define DEMO_SPI_LOOPBACK_DEV "spi0_0"
#define DEMO_SPI_LOOPBACK_DATA_WIDTH 8
#define DEMO_SPI_LOOPBACK_MODE (RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB)
#define DEMO_SPI_LOOPBACK_BAUD 5000000

/* i2c ds1339 demo */
#define DEMO_I2C_DS1339_DEV "mio_i2c9"

/* mac demo */
#define DEMO_MAC_DEV "xmac3"
#define DEMO_MAC_LINK_STATUS

/******************* functions *******************/

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

void gpio_demo(void);
void uart_demo(void);
void timer_demo(void);
void can_demo(void);
void spi_loopback_demo(void);
void i2c_ds1339_demo(void);
void smp_demo(void);
void mac_demo(void);
void localbus_demo(void);
void Lwip_Udp_Demo(void);
int Lwip_Tcp_Demo(void);
void fpga_intr_demo(void);

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
