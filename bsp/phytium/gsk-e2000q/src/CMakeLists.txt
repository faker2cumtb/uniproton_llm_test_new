set(SRCS start.S hwi_init.c print.c timer.c hwi_router.c osmain.c mmu_table.c)

add_library(src OBJECT ${SRCS})
