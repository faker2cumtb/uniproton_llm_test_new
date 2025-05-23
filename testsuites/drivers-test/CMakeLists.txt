include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB ALL_driver_SRC
    ./*.c)

if (${APP} STREQUAL "gpio-test")
    set(DEF_TEST "GPIO_TEST")
elseif(${APP} STREQUAL "timer-test")
    set(DEF_TEST "TIMER_TEST")
elseif(${APP} STREQUAL "i2c-test")
    set(DEF_TEST "I2C_TEST")
elseif(${APP} STREQUAL "smp-test")
    set(DEF_TEST "SMP_TEST")
elseif(${APP} STREQUAL "localbus-test")
    set(DEF_TEST "LOCALBUS_TEST")
else()
    return()
endif()

add_library(driversTest OBJECT ${ALL_driver_SRC})
target_compile_definitions(driversTest PUBLIC ${DEF_TEST})


    

