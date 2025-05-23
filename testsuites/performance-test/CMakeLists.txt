include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/kernel/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../rhealstone/support)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../rhealstone/support)

if (${APP} STREQUAL "stress_hrtimer_test")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/hrtimer/stress_hrtimer_test.c ${CMAKE_CURRENT_SOURCE_DIR}/hrtimer/nostress_hrtimer_test.c)
    set(DEF_TEST "STRESS_HRTIMER")
elseif(${APP} STREQUAL "nostress_hrtimer_test")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/hrtimer/nostress_hrtimer_test.c)
    set(DEF_TEST "NOSTRESS_HRTIMER")
elseif(${APP} STREQUAL "release_mutex_time_test")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/sem_mutex/release_mutex_time.c)
    set(DEF_TEST "RELEASE_MUTEX_TIME")
elseif(${APP} STREQUAL "interrupt_gpio_test")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/interrupt/interrupt_gpio.c ${CMAKE_CURRENT_SOURCE_DIR}/interrupt/stress_interrupt_test.c)
    set(DEF_TEST "INTERRUPT_GPIO")
elseif(${APP} STREQUAL "task_switch_stress")
    file(GLOB ALL_SRC
        ./task-switch-stress/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../rhealstone/support/*.c
        )
    set(DEF_TEST "TASK_SWITCH")
else()
    return()
endif()

add_library(performanceTest OBJECT ${ALL_SRC} ./performance_public.c)
target_compile_definitions(performanceTest PUBLIC ${DEF_TEST})
# target_include_directories(performanceTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../rhealstone/support)


