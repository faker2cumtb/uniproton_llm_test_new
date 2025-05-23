cmake_minimum_required(VERSION 3.14.1) # cmake最低版本要求

project(lwip LANGUAGES C ASM) # 项目名字为lwip

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)
set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdinc -nostdlib")

set(CMAKE_C_FLAGS "${CC_OPTION}")

############################## 外部传入变量 ##############################
set(HOME_PATH ../..)
set(INSTALL_DIR "$ENV{INSTALL_DIR}")

set(LWIP_SRC_PATH ${HOME_PATH}/src/net/lwip)

##############################include 头文件##############################
# 添加头文件
include_directories(
	# 安全函数头文件
	${HOME_PATH}/platform/libboundscheck/include

	# os头文件
	${HOME_PATH}/build/uniproton_config/config_armv8_${CPU_TYPE}
	${CMAKE_CURRENT_SOURCE_DIR}/../../config
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
	${HOME_PATH}/src/arch/include
	${HOME_PATH}/src/core/extend/lp/include
	${HOME_PATH}/src/core/ipc/include
	${HOME_PATH}/src/core/kernel/include
	${HOME_PATH}/src/include/uapi
	${HOME_PATH}/src/jsf/include
	${HOME_PATH}/src/mem/include
	${HOME_PATH}/src/om/include
	${HOME_PATH}/src/sandbox/include
	${HOME_PATH}/src/security/include
	${HOME_PATH}/src/utility/lib/include
	${HOME_PATH}/src/osal/linux/include_self_def

	# lwip头文件
	${LWIP_SRC_PATH}/src/include
	${HOME_PATH}/src/net/lwip_port
	# ${HOME_PATH}/src/net/adapter/include

	${HOME_PATH}/src/libc/musl/include
	${HOME_PATH}/src/libc/litelibc/include

	${CMAKE_CURRENT_SOURCE_DIR}
)

##############################添加要编译的.c文件##############################
file(GLOB_RECURSE LWIP_SRC  ${LWIP_SRC_PATH}/src/api/*.c
							${LWIP_SRC_PATH}/src/core/*.c
							${LWIP_SRC_PATH}/src/core/ipv4/*.c
							${LWIP_SRC_PATH}/src/core/ipv6/*.c
							${LWIP_SRC_PATH}/src/netif/*.c
							${LWIP_SRC_PATH}/src/netif/ppp/*.c
							${LWIP_SRC_PATH}/src/netif/ppp/polarssl/*.c)

# 添加要编译的.c文件
set(LWIP_SRCS
	# lwip port
	${HOME_PATH}/src/net/adapter/src/sys_arch.c

	# lwip-2.1.3
	${LWIP_SRC}
)

##############################设置各个平台lwip库的名字############
set(LIB_LWIP_NAME "lwip")

#编译生成lwip.a库
add_library(${LIB_LWIP_NAME} STATIC ${LWIP_SRCS})

#安装lwip.a库
install(TARGETS ${LIB_LWIP_NAME} ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

#拷贝头文件到output目录
file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/arpa/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/arpa)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/net/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/net)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/sys/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/sys)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/stdc/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/stdc)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/apps/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/apps)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/priv/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/priv)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/prot/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/prot)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/ppp/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif/ppp)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/ppp/polarssl/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif/ppp/polarssl)