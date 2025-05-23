set(SRCS
    drv_common.c
    drv_gpio.c
    drv_uart.c
    drv_i2c.c
    drv_spi.c
    drv_can.c
    drv_timer.c
    drv_xmac.c
    drv_localbus.c
    )
    
add_library(drv OBJECT ${SRCS})