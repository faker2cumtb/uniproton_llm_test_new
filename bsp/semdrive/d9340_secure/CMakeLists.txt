cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(OUTPUT_PATH ${HOME_PATH}/output)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)

set(CMAKE_C_FLAGS "-g -O0 -mthumb -D__thumb__ -mcpu=cortex-r5 -std=gnu11 -fno-common -fomit-frame-pointer -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16  -Wa,-mimplicit-it=thumb  -fstack-protector-strong  -funsigned-char -fdata-sections -ffunction-sections -fshort-enums")
set(CMAKE_CXX_FLAGS "-mthumb -D__thumb__ -mcpu=cortex-r5 -mfloat-abi=softfp -mfpu=fpv4-sp-d16  -fno-threadsafe-statics -fno-builtin -DEIGEN_NO_IO=1")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS})

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_cortex_r5_d9340_secure/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d9340_secure
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/include
    ${HOME_PATH}/src/utility/lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/output/libc/include
    ${OUTPUT_PATH}/libboundscheck/include
    ${OUTPUT_PATH}/libc/include
    ${OUTPUT_PATH}/UniProton/include
    ${HOME_PATH}/components/rtdriver/common/include
    ${HOME_PATH}/components/rtdriver/drivers/include
)
set(ldfile "${CMAKE_CURRENT_SOURCE_DIR}/build/d9340_secure.ld")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ${ldfile}")

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libD9340Secure.a"
    -Wl,--end-group
)
link_libraries(-lgcc)

add_subdirectory(src)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(${HOME_PATH}/components/rtdriver rtdriver)

# add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
# list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:src> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:mica>)
list(APPEND OBJS $<TARGET_OBJECTS:drivers> $<TARGET_OBJECTS:rtdriver> $<TARGET_OBJECTS:src> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:apps>)
add_executable(${APP} ${OBJS})
