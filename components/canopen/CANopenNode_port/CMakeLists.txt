cmake_minimum_required(VERSION 3.12)

project(canopen LANGUAGES C ASM)

set(LIB_NAME canopen)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)

set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(INSTALL_LIB_DIR ${BUILD_DIR}/../libs/${COMPILE_MODE})
set(INSTALL_INC_DIR ${BUILD_DIR}/../include/${LIB_NAME})

set(CANOPEN_ORIG_PATH ${HOME_PATH}/components/canopen/CANopenNode)

set(SRCS
    CO_app_target.c
    CO_driver_target.c
    OD.c
    ${CANOPEN_ORIG_PATH}/CANopen.c
    ${CANOPEN_ORIG_PATH}/301/CO_Emergency.c
    ${CANOPEN_ORIG_PATH}/301/CO_fifo.c
    ${CANOPEN_ORIG_PATH}/301/CO_HBconsumer.c
    ${CANOPEN_ORIG_PATH}/301/CO_NMT_Heartbeat.c
    ${CANOPEN_ORIG_PATH}/301/CO_Node_Guarding.c
    ${CANOPEN_ORIG_PATH}/301/CO_ODinterface.c
    ${CANOPEN_ORIG_PATH}/301/CO_PDO.c
    ${CANOPEN_ORIG_PATH}/301/CO_SDOclient.c
    ${CANOPEN_ORIG_PATH}/301/CO_SDOserver.c
    ${CANOPEN_ORIG_PATH}/301/CO_SYNC.c
    ${CANOPEN_ORIG_PATH}/301/CO_TIME.c
    ${CANOPEN_ORIG_PATH}/303/CO_LEDs.c
    ${CANOPEN_ORIG_PATH}/305/CO_LSSmaster.c
    ${CANOPEN_ORIG_PATH}/305/CO_LSSslave.c
)

set(LIB_HDRS
    CO_app_target.h
    CO_driver_target.h
    OD.h
    ${CANOPEN_ORIG_PATH}/CANopen.h
    ${CANOPEN_ORIG_PATH}/301/CO_config.h
    ${CANOPEN_ORIG_PATH}/301/CO_driver.h
    ${CANOPEN_ORIG_PATH}/301/CO_Emergency.h
    ${CANOPEN_ORIG_PATH}/301/CO_fifo.h
    ${CANOPEN_ORIG_PATH}/301/CO_HBconsumer.h
    ${CANOPEN_ORIG_PATH}/301/CO_NMT_Heartbeat.h
    ${CANOPEN_ORIG_PATH}/301/CO_Node_Guarding.h
    ${CANOPEN_ORIG_PATH}/301/CO_ODinterface.h
    ${CANOPEN_ORIG_PATH}/301/CO_PDO.h
    ${CANOPEN_ORIG_PATH}/301/CO_SDOclient.h
    ${CANOPEN_ORIG_PATH}/301/CO_SDOserver.h
    ${CANOPEN_ORIG_PATH}/301/CO_SYNC.h
    ${CANOPEN_ORIG_PATH}/301/CO_TIME.h
    ${CANOPEN_ORIG_PATH}/303/CO_LEDs.h
    ${CANOPEN_ORIG_PATH}/304/CO_GFC.h
    ${CANOPEN_ORIG_PATH}/304/CO_SRDO.h
    ${CANOPEN_ORIG_PATH}/305/CO_LSS.h
    ${CANOPEN_ORIG_PATH}/305/CO_LSSmaster.h
    ${CANOPEN_ORIG_PATH}/305/CO_LSSslave.h
    ${CANOPEN_ORIG_PATH}/309/CO_gateway_ascii.h
    ${CANOPEN_ORIG_PATH}/extra/CO_trace.h
)

add_library(${LIB_NAME} STATIC ${SRCS})

target_include_directories(${LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
           ${CANOPEN_ORIG_PATH}
           ${BUILD_DIR}/../include
           ${BUILD_DIR}/../include/src
           ${BUILD_DIR}/../include/rtdriver
)

install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
foreach (_LIB_HDR ${LIB_HDRS})
    set(_HDR_FILE ${_LIB_HDR})
    string(REGEX REPLACE "^${CANOPEN_ORIG_PATH}/" "" _HDR_FILE ${_HDR_FILE})
    string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}/" "" _HDR_FILE ${_HDR_FILE})
    get_filename_component(_SUB_DIR ${_HDR_FILE} DIRECTORY)
    install(FILES ${_LIB_HDR} DESTINATION ${INSTALL_INC_DIR}/${_SUB_DIR})
endforeach()
