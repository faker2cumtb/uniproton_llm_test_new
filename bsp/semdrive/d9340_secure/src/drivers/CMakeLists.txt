set(SRCS
    watchdog/sdrv_watchdog_drv.c
    watchdog/sdrv_watchdog.c
    watchdog/watchdog.c
    timer/sdrv_tmr.c
    )
    
add_library(drivers OBJECT ${SRCS})