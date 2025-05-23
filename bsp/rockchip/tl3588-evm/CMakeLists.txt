cmake_minimum_required(VERSION 3.12)
project(${APP} LANGUAGES C ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ld)

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdinc -nostdlib")
set(AS_OPTION "-g -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/${CPU_TYPE}.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/${CPU_TYPE}.ld")

add_compile_options(
    -Wno-parentheses
)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_${CPU_TYPE}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arch
    ${CMAKE_CURRENT_SOURCE_DIR}/include/net
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utility

    ${CMAKE_CURRENT_SOURCE_DIR}/../common/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/drv
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/gmac
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/pcie
   
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/apps

    ${HOME_PATH}/components/rtdriver/common/include
    ${HOME_PATH}/components/rtdriver/drivers/include
)

add_subdirectory(config)
add_subdirectory(src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/hal hal)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/drv drv)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/gmac gmac)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/rk3588/pcie pcie)
add_subdirectory(${HOME_PATH}/components/rtdriver rtdriver)
add_subdirectory(${HOME_PATH}/components/mica mica)

list(APPEND OBJS $<TARGET_OBJECTS:config>)
list(APPEND OBJS $<TARGET_OBJECTS:src>)
list(APPEND OBJS $<TARGET_OBJECTS:hal>)
list(APPEND OBJS $<TARGET_OBJECTS:drv>)
list(APPEND OBJS $<TARGET_OBJECTS:gmac>)
list(APPEND OBJS $<TARGET_OBJECTS:pcie>)
list(APPEND OBJS $<TARGET_OBJECTS:rtdriver>)
list(APPEND OBJS $<TARGET_OBJECTS:mica>)

add_library(${BSP_LIB} STATIC ${OBJS})
set(INSTALL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/${COMPILE_MODE})
install(TARGETS ${BSP_LIB} ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
