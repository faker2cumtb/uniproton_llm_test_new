ENTRY(OsElxState)

_stack_size = 0x10000;
_heap_size = 0x10000;

MEMORY
{
    IMU_SRAM (rwx) : ORIGIN = 0x4a000000, LENGTH = 224*1024*1024
    MMU_MEM (rwx) : ORIGIN = 0x58000000, LENGTH = 32*1024*1024
}

SECTIONS
{
    __os_section_start = .;
    text_start = .;
    .start_bspinit :
    {
        __text_start = .;
        KEEP(*(.text.bspinit))
    } > IMU_SRAM

    .start_text :
    {
        KEEP(*(.text.startup))
    } > IMU_SRAM

    .text :
    {
        *(.text)
        *(.text.*)
        *(*.text)
        __os_stub_text_start = .;
        *(.stub.text)
        __os_stub_text_end = .;
        . = ALIGN(8);
        __text_end = .;
    } > IMU_SRAM
    . = ALIGN(4096);
    data_copy_start = .;
    __protect_len = data_copy_start - ORIGIN(IMU_SRAM);
    __non_protect_len = ORIGIN(MMU_MEM) + LENGTH(MMU_MEM) - data_copy_start;

    .rodata :
    {
        . = ALIGN(4096);
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    } > IMU_SRAM

    .eh_frame :
    {
        . = ALIGN(8);
        __os_unwind_table_start = .;
        *(.eh_frame)
        __os_unwind_table_end = .;
    } > IMU_SRAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE (__HEAP_INIT = .);
        . = . + _heap_size;
        . = ALIGN(8);
        PROVIDE (__HEAP_END = .);
    } > IMU_SRAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE (__os_sys_sp_start = .);
        . = . + _stack_size;
        . = ALIGN(8);
        PROVIDE (__os_sys_sp_end = .);
    } > IMU_SRAM
    end = .;

    .percpu.data :
    {
        __os_per_cpu_start = .;
        *(.os.percpu.data)
        __os_per_cpu_end = .;
        LONG (ALIGNOF(.percpu.data))
    } > IMU_SRAM

    .data :
    {
        . = ALIGN(8);
        __data_start = .;
        *(*.data)
        *(.data.*)
        __os_stub_data_start = .;
        *(.stub.data)
        __os_stub_data_end = .;
        . = ALIGN(8);
        __os_text_start = .;
        QUAD(__text_start)
        QUAD(__text_end)
        __os_text_end = .;
        __data_end = .;
    } > IMU_SRAM

    .resource_table :
    {
        . = ALIGN(8);
        KEEP(*(.resource_table));
    } > IMU_SRAM

    .llt.bss :
    {
        _llt_bss_start = .;
        *__code_measure_stub*.o(.bss)
        *__code_measure_stub*.o(.bss.*)
        _llt_bss_end = .;
    } > IMU_SRAM

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        *(*.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
    } > IMU_SRAM
    __os_section_end = .;

    .mmu.table.base :
    {
        PROVIDE (g_mmu_page_begin = .);
        PROVIDE (g_mmu_page_end = g_mmu_page_begin + 32*1024*1024);
    } > MMU_MEM
    __mmu_len = g_mmu_page_end - g_mmu_page_begin;
}
