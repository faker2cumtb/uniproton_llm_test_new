if (NOT ${APP} STREQUAL "UniProton_test_libxml2_interface")
    return()
endif()

set(BUILD_APP "UniProton_test_libxml2_interface")
set(ALL_SRC xmltest.c)

if(${CPU_TYPE} STREQUAL "x86_64" OR ${CPU_TYPE} STREQUAL "hi3093")
    add_library(libxml2Test OBJECT ${ALL_SRC})
endif()