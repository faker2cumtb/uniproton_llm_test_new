cmake_minimum_required(VERSION 3.12)
project(${APP} LANGUAGES C ASM)

string(TOUPPER ${CPU_TYPE} CPU_TYPE_UPPER)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(BOARD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(DEMO_PATH ${HOME_PATH}/demos/common)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ld)

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -Wl,--gc-sections -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdinc -nostdlib")
set(AS_OPTION "-g -Wl,--gc-sections -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${BOARD_PATH}/build/${CPU_TYPE}.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${BOARD_PATH}/build/${CPU_TYPE}.ld")

add_compile_options(
    -Wno-parentheses
)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_${CPU_TYPE}/defconfig)

set(SRC main.c)

list(APPEND SRC
    ${DEMO_PATH}/demo_gpio.c
    ${DEMO_PATH}/demo_timer.c
    ${DEMO_PATH}/demo_uart.c
    ${DEMO_PATH}/demo_smp.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMPONENT_INCS}
    ${BOARD_PATH}/include
    ${BOARD_PATH}/include/arch
    ${BOARD_PATH}/include/hal
    ${BOARD_PATH}/include/drv
    ${BOARD_PATH}/include/src
    ${BOARD_PATH}/include/net
    ${BOARD_PATH}/include/rtdriver
    ${BOARD_PATH}/include/utility
    ${BOARD_PATH}/include/kernel
)

add_compile_options(
    -Wno-parentheses
)

link_directories(${BOARD_PATH}/libs/${COMPILE_MODE})
link_libraries(
    -Wl,--start-group
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libCortexMXsec_c.lib
    ${BOARD_PATH}/libs/${COMPILE_MODE}/lib${CPU_TYPE_UPPER}.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/lib${CPU_TYPE}Bsp.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libmetal.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libopen_amp.a
    ${COMPONENT_LIBS}
    -Wl,--end-group
    -lgcc
)

add_library(mainapp OBJECT ${SRC})

list(APPEND OBJS $<TARGET_OBJECTS:mainapp>)

set(OBJS_GLOBAL ${OBJS})
set(CREATE_EXECUTABLE TRUE)

include(${HOME_PATH}/testsuites/cmake/AddTestSuite.cmake)
if(SKIP_GEN_BINARY)
    return()
endif()

if (CREATE_EXECUTABLE)
    add_executable(${APP} ${OBJS})
endif()
