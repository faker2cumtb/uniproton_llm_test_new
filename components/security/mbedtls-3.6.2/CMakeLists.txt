cmake_minimum_required(VERSION 3.12)

project(mbedtls LANGUAGES C ASM)

set(LIB_NAME mbedtls)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)


set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(INSTALL_LIB_DIR ${BUILD_DIR}/../libs/${COMPILE_MODE})
set(INSTALL_INC_DIR ${BUILD_DIR}/../include/${LIB_NAME})

############################## 外部传入变量 ##############################

include_directories(
    ${HOME_PATH}/components/security/mbedtls-3.6.2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mbedtls
    ${BUILD_DIR}/../include
)

add_subdirectory(library)
list(APPEND OBJS $<TARGET_OBJECTS:mbedtls_src>)

add_library(${LIB_NAME} STATIC ${OBJS})

#安装mbedtls.a库
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
