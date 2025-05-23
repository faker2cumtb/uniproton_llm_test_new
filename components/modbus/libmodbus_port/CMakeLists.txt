cmake_minimum_required(VERSION 3.12)

project(modbus LANGUAGES C ASM)

set(LIB_NAME modbus)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)

set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(INSTALL_LIB_DIR ${BUILD_DIR}/../libs/${COMPILE_MODE})
set(INSTALL_INC_DIR ${BUILD_DIR}/../include/${LIB_NAME})

set(MODBUS_ORIG_PATH ${HOME_PATH}/components/modbus/libmodbus)

set(SRCS
    modbus_port.c
    ${MODBUS_ORIG_PATH}/src/modbus.c
    ${MODBUS_ORIG_PATH}/src/modbus-data.c
    ${MODBUS_ORIG_PATH}/src/modbus-rtu.c
    ${MODBUS_ORIG_PATH}/src/modbus-tcp.c
)

set(LIB_HDRS
    modbus_port.h
    modbus_port-rtu.h
    modbus_port-tcp.h
    ${MODBUS_ORIG_PATH}/src/modbus.h
    ${MODBUS_ORIG_PATH}/src/modbus-rtu.h
    ${MODBUS_ORIG_PATH}/src/modbus-tcp.h
    ${MODBUS_ORIG_PATH}/src/modbus-version.h
)

add_library(${LIB_NAME} STATIC ${SRCS})

target_compile_definitions(${LIB_NAME} PRIVATE HAVE_STRLCPY)

target_include_directories(${LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
           ${MODBUS_ORIG_PATH}
           ${BUILD_DIR}/../include
           ${BUILD_DIR}/../include/src
           ${BUILD_DIR}/../include/net
           ${BUILD_DIR}/../include/rtdriver
)

install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
install(FILES ${LIB_HDRS} DESTINATION ${INSTALL_INC_DIR})
