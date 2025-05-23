if (NOT ${APP} STREQUAL "UniProton_test_log_interface")
    return()
endif()

set(BUILD_APP "UniProton_test_log_interface")
set(ALL_SRC log_test.c)

add_library(logTest OBJECT ${ALL_SRC})