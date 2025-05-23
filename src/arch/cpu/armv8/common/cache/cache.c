#include <stdio.h>
#include <stdlib.h>
#include "prt_typedef.h"

#define BIT_MASK(n) ((1 << (n)) - 1)
#define	CTR_EL0_DMINLINE_SHIFT		16
#define	CTR_EL0_DMINLINE_MASK		BIT_MASK(4)

#define dc_ops(op, val)							\
({									\
	__asm__ volatile ("dc " op ", %0" :: "r" (val) : "memory");	\
})
#define ic_ops(op, val)							\
({									\
	__asm__ volatile ("ic " op ", %0" :: "r" (val) : "memory");	\
})

#define barrier_dsync_fence_full()      \
({                                      \
	__asm__ volatile ("dsb sy" ::: "memory");               \
})

static size_t dcache_line_size = 0;

static inline size_t arch_dcache_line_size_get(void)
{
	uint64_t ctr_el0;
	uint32_t dminline;

	if (dcache_line_size) {
		return dcache_line_size;
	}

    OS_EMBED_ASM("MRS %0, CTR_EL0" : "=r"(ctr_el0) : : "memory", "cc");

	dminline = (ctr_el0 >> CTR_EL0_DMINLINE_SHIFT) & CTR_EL0_DMINLINE_MASK;

	dcache_line_size = 4 << dminline;

	return dcache_line_size;
}

void cache_invalid_by_range(void *addr, size_t size)
{
    size_t line_size;
	uintptr_t start_addr = (uintptr_t)addr;
	uintptr_t end_addr = start_addr + size;

    line_size = arch_dcache_line_size_get();

    if (end_addr & (line_size - 1)) {
        end_addr &= ~(line_size - 1);
        dc_ops("civac", end_addr);
    }

    if (start_addr & (line_size - 1)) {
        start_addr &= ~(line_size - 1);
        if (start_addr == end_addr) {
            goto done;
        }
        dc_ops("civac", start_addr);
        start_addr += line_size;
    }

    start_addr &= ~(line_size - 1);
	while (start_addr < end_addr) {
		dc_ops("ivac", start_addr);
		start_addr += line_size;
	}

done:
	barrier_dsync_fence_full();
}

void cache_flush_by_range(void *addr, size_t size)
{
    size_t line_size;
	uintptr_t start_addr = (uintptr_t)addr;
	uintptr_t end_addr = start_addr + size;

    line_size = arch_dcache_line_size_get();

    start_addr &= ~(line_size - 1);
	while (start_addr < end_addr) {
		dc_ops("cvac", start_addr);
		start_addr += line_size;
	}

done:
	barrier_dsync_fence_full();
}