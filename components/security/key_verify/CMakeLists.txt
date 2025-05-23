cmake_minimum_required(VERSION 3.12)

project(key_verify LANGUAGES C ASM)

set(LIB_NAME key_verify)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)


set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(INSTALL_LIB_DIR ${BUILD_DIR}/../libs/${COMPILE_MODE})
set(INSTALL_INC_DIR ${BUILD_DIR}/../include/${LIB_NAME})

############################## 外部传入变量 ##############################

set(VERYFY_SRC_PATH ${HOME_PATH}/components/security/key_verify)

include_directories(
    ${HOME_PATH}/components/security/mbedtls-3.6.2/include
)
set(SRCS
    ${VERYFY_SRC_PATH}/algorithm.c
    ${VERYFY_SRC_PATH}/key_verify.c
    ${VERYFY_SRC_PATH}/boardid.c
)

set(LIB_HDRS
    key_verify.h
)

add_library(${LIB_NAME} STATIC ${SRCS})

target_include_directories(${LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
           ${BUILD_DIR}/../include
)

#安装key_verify.a库
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
install(FILES ${LIB_HDRS} DESTINATION ${INSTALL_INC_DIR})
