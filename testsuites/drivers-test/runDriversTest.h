#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H
#define TO_STRING(x) #x
extern void gpio_demo();
extern void uart_demo();
extern void timer_demo();
extern void can_demo();
extern void spi_demo();
extern void i2c_ds1339_demo();
extern void smp_demo();
extern void xmac_demo();
extern void localbus_demo();
extern void Udp_Demo();
extern void Tcp_Demo();
extern void ethercat_demo();
extern void canopen_demo();
extern void Modbus_Client_RTU_Demo();
extern void Modbus_Server_RTU_Demo();
extern void Modbus_Client_TCP_Demo();
extern void Modbus_Server_TCP_Demo();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
#ifdef GPIO_TEST
    gpio_demo,
#endif
#ifdef TIMER_TEST
    timer_demo,
#endif
#ifdef I2C_TEST
    i2c_ds1339_demo,
#endif
#ifdef SMP_TEST
    smp_demo,
#endif
#ifdef LOCALBUS_TEST
    localbus_demo,
#endif
};


char run_test_name_1[][50] = {
    #ifdef GPIO_TEST
    "gpio_demo",
#endif
#ifdef TIMER_TEST
    "timer_demo",
#endif
#ifdef I2C_TEST
    "i2c_ds1339_demo",
#endif
#ifdef SMP_TEST
    "smp_demo",
#endif
#ifdef LOCALBUS_TEST
    "localbus_demo",
#endif
};

#endif
