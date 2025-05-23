#ifndef __PRT_CACHE_EXTERNAL_H__
#define __PRT_CACHE_EXTERNAL_H__

#include "prt_buildef.h"
#include <stddef.h>

extern void os_asm_invalidate_dcache_all(void);
extern void os_asm_invalidate_icache_all(void);
extern void os_asm_invalidate_tlb_all(void);

#if defined(OS_ARCH_ARMV8)
extern void cache_flush_by_range(void *addr, size_t size);
extern void cache_invalid_by_range(void *addr, size_t size);
#else
#define cache_flush_by_range(addr, size) os_asm_invalidate_dcache_all()
#define cache_invalid_by_range(addr, size) os_asm_flush_dcache_all()
#endif

#endif /* __PRT_CACHE_EXTERNAL_H__ */