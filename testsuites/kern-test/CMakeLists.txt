include_directories(
    ${HOME_PATH}/src/om/include
    ${HOME_PATH}/src/mem/include
)

if (${APP} STREQUAL "UniProton_test_sem")
    set(BUILD_APP "UniProton_test_sem")
    set(ALL_SRC sem_mutex_test.c kern_test_public.c)
endif()

if (${APP} STREQUAL "UniProton_test_rr_sched")
    set(BUILD_APP "UniProton_test_rr_sched")
    set(ALL_SRC sched_rr_test.c kern_test_public.c)
endif()

if (${APP} STREQUAL "UniProton_test_priority")
    set(BUILD_APP "UniProton_test_priority")
    set(ALL_SRC priority_test.c kern_test_public.c)
endif()

if (${APP} STREQUAL "UniProton_test_mmu")
    if(${CONFIG_OS_ARCH_ARMV8})
        set(BUILD_APP "UniProton_test_mmu")
        set(ALL_SRC mmu_test.c kern_test_public.c)
    else()
        return()
    endif()
endif()

if (${APP} STREQUAL "UniProton_test_ir")
    if(${CONFIG_OS_ARCH_ARMV8})
        set(BUILD_APP "UniProton_test_ir")
        set(ALL_SRC interrupt_test.c kern_test_public.c)
    else()
        return()
    endif()
endif()

if (${APP} STREQUAL "UniProton_test_spinlock")
    set(ALL_SRC spinlock_test.c kern_test_public.c)
endif()

add_library(kernTest OBJECT ${ALL_SRC})
