include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB ALL_SRC
    ./hooktest/*.c
    ./*.c)

if (${APP} STREQUAL "hook-test")
    set(BUILD_APP "hook-test")
else()
    return()
endif()
 
add_library(hookTest OBJECT ${ALL_SRC})
