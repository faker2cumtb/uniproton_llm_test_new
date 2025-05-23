# 此文件用于添加测试套件

if (${APP} MATCHES "UniProton_test_posix" OR
    ${APP} STREQUAL "UniProton_test_proxy_posix_interface")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance conformance)
    add_subdirectory(${HOME_PATH}/testsuites/shell-test shell-test)
    add_subdirectory(${HOME_PATH}/testsuites/libc-test libc-test)
    if (${APP} STREQUAL "UniProton_test_posix_math_interface" OR 
         ${APP} STREQUAL "UniProton_test_posix_exit_interface")
        set(SKIP_GEN_BINARY TRUE)
        return()
    endif()
    list(APPEND OBJS $<TARGET_OBJECTS:posixTest>)
elseif(${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:rhealstoneTest>)
elseif(${APP} STREQUAL "gpio-test" OR 
        ${APP} STREQUAL "timer-test" OR
        ${APP} STREQUAL "i2c-test" OR
        ${APP} STREQUAL "smp-test" OR 
        ${APP} STREQUAL "localbus-test")
    add_subdirectory(${HOME_PATH}/testsuites/drivers-test tmp)
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    list(APPEND OBJS $<TARGET_OBJECTS:driversTest>)
elseif(${APP} STREQUAL "ethercatTest")
    add_subdirectory(ethercat)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat>)
elseif(${APP} STREQUAL "UniProton_test_sem" OR
    ${APP} STREQUAL "UniProton_test_rr_sched" OR
    ${APP} STREQUAL "UniProton_test_mmu" OR
    ${APP} STREQUAL "UniProton_test_ir" OR
    ${APP} STREQUAL "UniProton_test_priority" OR 
    ${APP} STREQUAL "UniProton_test_spinlock")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/kern-test kern-test)
    list(APPEND OBJS $<TARGET_OBJECTS:kernTest>)
elseif(${APP} STREQUAL "fs-test")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/fs-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:fsTest>)
elseif(${APP} STREQUAL "hook-test")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/hook-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:hookTest>)
elseif(${APP} STREQUAL "stress_hrtimer_test" OR
    ${APP} STREQUAL "nostress_hrtimer_test" OR 
    ${APP} STREQUAL "release_mutex_time_test" OR 
    ${APP} STREQUAL "interrupt_gpio_test" OR 
    ${APP} STREQUAL "task_switch_stress")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/performance-test performance)
    list(APPEND OBJS $<TARGET_OBJECTS:performanceTest>)
elseif(${APP} STREQUAL "net-test")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    add_subdirectory(${HOME_PATH}/testsuites/net-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:netTest>)
elseif(${APP} STREQUAL "cxxTest")
    target_compile_options(mainapp PUBLIC -DTESTSUITE_CASE)
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/cxx-test tmp)
        target_compile_options(cxxtest PUBLIC -D_POSIX_THREADS)
        list(APPEND OBJS $<TARGET_OBJECTS:cxxtest>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_CXX AND CONFIG_OS_OPTION_LOCALE")
        return()
    endif()
endif()