include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB ALL_fs_SRC
    ./fs/*/*.c
    ./*.c)

if (${APP} STREQUAL "fs-test")
    set(BUILD_APP "fs-test")
else()
    return()
endif()

add_library(fsTest OBJECT ${ALL_fs_SRC})
# target_include_directories(rhealstoneTest PUBLIC ./support)




