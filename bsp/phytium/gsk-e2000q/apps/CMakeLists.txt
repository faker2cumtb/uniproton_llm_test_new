cmake_minimum_required(VERSION 3.12)
project(${APP} LANGUAGES C CXX ASM)

set(CREATE_EXECUTABLE TRUE)

string(TOUPPER ${CPU_TYPE} CPU_TYPE_UPPER)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(BOARD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(DEMO_PATH ${HOME_PATH}/demos/common)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ld)

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -Wl,--gc-sections -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables -nostdinc -nostdlib")
set(AS_OPTION "-g -Wl,--gc-sections -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--eh-frame-hdr")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -nostdinc++")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${BOARD_PATH}/build/${CPU_TYPE}.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${BOARD_PATH}/build/${CPU_TYPE}.ld")
set(CMAKE_CXX_STANDARD 11)

add_compile_options(
    -Wno-parentheses
)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_${CPU_TYPE}/defconfig)

file(GLOB_RECURSE ALL_SRC_CURR_DIR *.c)
if (CREATE_EXECUTABLE)
else()
list(REMOVE_ITEM ALL_SRC_CURR_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo_localbus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ethercat/hr_delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ethercat/igh_adapt_interface.c
)
endif()
set(SRC ${ALL_SRC_CURR_DIR})

if (CONFIG_OS_SUPPORT_CANOPEN)
    if (CREATE_EXECUTABLE)
    list(APPEND SRC
        ${DEMO_PATH}/demo_canopen.c
    )
    endif()
    list(APPEND COMPONENT_INCS ${BOARD_PATH}/include/canopen)
    list(APPEND COMPONENT_LIBS ${BOARD_PATH}/libs/${COMPILE_MODE}/libcanopen.a)
endif() # CONFIG_OS_SUPPORT_CANOPEN

if (CONFIG_OS_SUPPORT_MODBUS)
    if (CREATE_EXECUTABLE)
    list(APPEND SRC
        ${DEMO_PATH}/modbus/demo_modbus_client_rtu.c
        ${DEMO_PATH}/modbus/demo_modbus_server_rtu.c
        ${DEMO_PATH}/modbus/demo_modbus_client_tcp.c
        ${DEMO_PATH}/modbus/demo_modbus_server_tcp.c
    )
    endif()
    list(APPEND COMPONENT_INCS ${BOARD_PATH}/include/modbus)
    list(APPEND COMPONENT_LIBS ${BOARD_PATH}/libs/${COMPILE_MODE}/libmodbus.a)
endif() # CONFIG_OS_SUPPORT_MODBUS

if (CONFIG_OS_SUPPORT_IGH_ETHERCAT)
    if (CREATE_EXECUTABLE)
    list(APPEND SRC
        ethercat/igh_adapt_interface.c
        ethercat/hr_delay.c
        ${DEMO_PATH}/ethercat/demo_ethercat.c
    )
    endif()
    list(APPEND COMPONENT_INCS ethercat)
endif() # CONFIG_OS_SUPPORT_IGH_ETHERCAT

if (CONFIG_OS_OPTION_MICA)
    if (CREATE_EXECUTABLE)
    list(APPEND SRC
        ${DEMO_PATH}/mica/demo_powerdown_protect.c
        ${DEMO_PATH}/mica/demo_log.c
    )
    endif()
endif() # CONFIG_OS_OPTION_MICA

if (CREATE_EXECUTABLE)
list(APPEND SRC
    ${DEMO_PATH}/demo_gpio.c
    ${DEMO_PATH}/demo_i2c_ds1339.c
    ${DEMO_PATH}/demo_spi_loopback.c
    ${DEMO_PATH}/demo_timer.c
    ${DEMO_PATH}/demo_uart.c
    ${DEMO_PATH}/demo_mac.c
    ${DEMO_PATH}/demo_lwip.c
    ${DEMO_PATH}/demo_smp.c
    ${DEMO_PATH}/demo_mbedtls.c
    ${DEMO_PATH}/demo_verify.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo_localbus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo_fpga_intr.c
)
endif()

if (CONFIG_OS_SUPPORT_CXX)
    include_directories(
        ${TOOLCHAIN_PATH}/aarch64-openeuler-linux-gnu/include/c++/10.3.1
        ${BOARD_PATH}/include/libcxx
    )
endif() # CONFIG_OS_SUPPORT_CXX

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
    ${BOARD_PATH}/include/user
    ${BOARD_PATH}/include/om
    ${BOARD_PATH}/include/mem
)

if (CONFIG_SECURITY_MBEDTLS)
    include_directories(
        ${BOARD_PATH}/include/mbedtls
    )
endif() # CONFIG_SECURITY_MBEDTLS

add_compile_options(
    -Wno-parentheses
)

link_directories(${BOARD_PATH}/libs/${COMPILE_MODE})

file(GLOB ALL_USER_LIBS ${BOARD_PATH}/libs/user/*.a)
message("INFO: ALL_USER_LIBS=(${ALL_USER_LIBS})")
set(LINK_LIBS
    -Wl,--start-group
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libCortexMXsec_c.lib
    ${BOARD_PATH}/libs/${COMPILE_MODE}/lib${CPU_TYPE_UPPER}.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/lib${CPU_TYPE}Bsp.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/liblwip.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libmetal.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libopen_amp.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libmbedtls.a
    ${BOARD_PATH}/libs/${COMPILE_MODE}/libkey_verify.a
    ${ALL_USER_LIBS}
    ${COMPONENT_LIBS}
)

if (CONFIG_OS_SUPPORT_CXX)
    list(APPEND LINK_LIBS
        ${BOARD_PATH}/libs/${COMPILE_MODE}/libstdc++.a
        ${BOARD_PATH}/libs/${COMPILE_MODE}/libsupc++.a
        "${TOOLCHAIN_PATH}/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/libgcc.a"
        "${TOOLCHAIN_PATH}/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/libgcc_eh.a"
    )
endif() # CONFIG_OS_SUPPORT_CXX

list(APPEND LINK_LIBS
    -Wl,--end-group
    -lgcc
)

link_libraries(${LINK_LIBS})

message("INFO: SRC=(${SRC})")
add_library(mainapp OBJECT ${SRC})

list(APPEND OBJS $<TARGET_OBJECTS:mainapp>)

set(OBJS_GLOBAL ${OBJS})

include(${HOME_PATH}/testsuites/cmake/AddTestSuite.cmake)
if(SKIP_GEN_BINARY)
    return()
endif()

message("INFO: OBJS=(${OBJS})")
if (CREATE_EXECUTABLE)
    add_executable(${APP} ${OBJS})
else()
    add_library(${APP} ${OBJS})
endif()
