set(RT_COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/src)
set(RT_DRIVERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

set(SRCS
    ${RT_COMMON_PATH}/device.c
    ${RT_COMMON_PATH}/intr.c
    ${RT_COMMON_PATH}/memory.c
    ${RT_COMMON_PATH}/mutex.c
    ${RT_COMMON_PATH}/semaphore.c

    ${RT_DRIVERS_PATH}/ipc/completion.c
    ${RT_DRIVERS_PATH}/ipc/ringbuffer.c

    ${RT_DRIVERS_PATH}/i2c/i2c_dev.c
    ${RT_DRIVERS_PATH}/i2c/i2c_core.c
    
    ${RT_DRIVERS_PATH}/spi/spi_dev.c
    ${RT_DRIVERS_PATH}/spi/spi_core.c
    ${RT_DRIVERS_PATH}/spi/qspi_core.c
    
    ${RT_DRIVERS_PATH}/serial/serial_v2.c
    ${RT_DRIVERS_PATH}/can/can.c
    ${RT_DRIVERS_PATH}/hwtimer/hwtimer.c
    ${RT_DRIVERS_PATH}/misc/pin.c
    )

add_library(rtdriver OBJECT ${SRCS})
