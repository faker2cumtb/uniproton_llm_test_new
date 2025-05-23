set(HAL_SRCS_DIR "../../hal")

set(SRCS
    hal_base.c
    hal_cru_rk3588.c
    hal_pinctrl.c
    ${HAL_SRCS_DIR}/hal_canfd.c
    ${HAL_SRCS_DIR}/hal_cru.c
    ${HAL_SRCS_DIR}/hal_gpio.c
    ${HAL_SRCS_DIR}/hal_i2c.c
    ${HAL_SRCS_DIR}/hal_spi.c
    ${HAL_SRCS_DIR}/hal_timer.c
    ${HAL_SRCS_DIR}/hal_uart.c
)

add_library(hal OBJECT ${SRCS})
