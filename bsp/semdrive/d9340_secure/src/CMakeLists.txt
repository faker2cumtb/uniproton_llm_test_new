set(SRCS hwi_init.c print.c timer.c)
add_library(src OBJECT ${SRCS})

add_subdirectory(drivers)
