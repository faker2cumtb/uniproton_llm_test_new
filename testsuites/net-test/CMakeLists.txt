include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB ALL_net_SRC
    ./*/*.c
    ./*.c)

if (${APP} STREQUAL "net-test")
    set(BUILD_APP "net-test")
else()
    return()
endif()

add_library(netTest OBJECT ${ALL_net_SRC})