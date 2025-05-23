#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_module.h"
#include "prt_errno.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "prt_atomic.h"
#include "prt_cache_external.h"
#include "prt_mmu_external.h"
#include "string.h"

#ifndef OS_MMU_PA_BITS
#error "OS_MMU_PA_BITS not defined"
#endif

#ifndef OS_MAX_XLAT_TABLES
#error "OS_MAX_XLAT_TABLES not defined"
#endif

extern U64 g_mmu_page_begin;
extern U64 g_mmu_page_end;

extern U32 PRT_Printf(const char *format, ...);

/* Only 4K granule is supported */
#define PAGE_SIZE_SHIFT		12U

#define OS_SEC_RSC_TABLE __attribute__((section(".data")))

static OS_SEC_RSC_TABLE mmu_ctrl_s g_mmu_ctrl = { 0 };
static OS_SEC_RSC_TABLE uint16_t xlat_use_count[OS_MAX_XLAT_TABLES] = { 0 };
static OS_SEC_RSC_TABLE struct PrtSpinLock mmu_spinLock;

//need to call spin lock before use
static void* alloc_new_table(void)
{
    uint64_t table;
    unsigned int i;

    /* Look for a free table. */
    for (i = 0U; i < OS_MAX_XLAT_TABLES; i++) {
        if (xlat_use_count[i] == 0U) {
            table = g_mmu_ctrl.tlb_addr + i * (1 << PAGE_SIZE_SHIFT);
            memset((void *)table, 0, 1 << PAGE_SIZE_SHIFT);
            xlat_use_count[i] = 1U;
            return (void *)table;
        }
    }

    PRT_Printf("OS_MAX_XLAT_TABLES too small");
    return NULL;
}

static inline unsigned int table_index(U64 pte)
{
    return (pte - g_mmu_ctrl.tlb_addr) / (1 << PAGE_SIZE_SHIFT);
}

/* Makes a table free for reuse. */
static void free_table(U64 table)
{
    unsigned int i = table_index(table);

    if(i >= OS_MAX_XLAT_TABLES || xlat_use_count[i] != 1U) {
        PRT_Printf("free_table: bad table %p\n", table);
        return;
    }

    xlat_use_count[i] = 0U;
}

/* Adjusts usage count and returns current count. */
static int table_usage(U64 pte, int adjustment)
{
    unsigned int i = table_index(pte);

    xlat_use_count[i] += adjustment;
    if(xlat_use_count[i] <= 0)
    {
        PRT_Printf("xlat_use_count should > 0");
    }
    
    return xlat_use_count[i];
}

static U64 mmu_get_tcr(U32 *pips, U32 *pva_bits)
{
    U64 ips, va_bits;
    U64 tcr;
    
    if (OS_MMU_PA_BITS > MMU_BITS_44) {
        ips = MMU_PHY_ADDR_LEVEL_5;
        va_bits = MMU_BITS_48;
    } else if (OS_MMU_PA_BITS > MMU_BITS_42) {
        ips = MMU_PHY_ADDR_LEVEL_4;
        va_bits = MMU_BITS_44;
    } else if (OS_MMU_PA_BITS > MMU_BITS_40) {
        ips = MMU_PHY_ADDR_LEVEL_3;
        va_bits = MMU_BITS_42;
    } else if (OS_MMU_PA_BITS > MMU_BITS_36) {
        ips = MMU_PHY_ADDR_LEVEL_2;
        va_bits = MMU_BITS_40;
    } else if (OS_MMU_PA_BITS > MMU_BITS_32) {
        ips = MMU_PHY_ADDR_LEVEL_1;
        va_bits = MMU_BITS_36;
    } else {
        ips = MMU_PHY_ADDR_LEVEL_0;
        va_bits = MMU_BITS_32;
    }
    
    tcr = TCR_EL1_RSVD | TCR_IPS(ips);
    
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
    } else {
        tcr |= TCR_TG0_64K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
    }
    
    tcr |= TCR_T0SZ(va_bits);
    
    if (pips != NULL) {
        *pips = ips;
    }
    
    if (pva_bits != NULL) {
        *pva_bits = va_bits;
    }
    
    return tcr;
}

static U32 mmu_get_pte_type(U64 const *pte)
{
    return (U32)(*pte & PTE_TYPE_MASK);
}

static U32 mmu_level2shift(U32 level)
{
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        return (U32)(MMU_BITS_12 + MMU_BITS_9 * (MMU_LEVEL_3 - level));
    } else {
        return (U32)(MMU_BITS_16 + MMU_BITS_13 * (MMU_LEVEL_3 - level));
    }
}

static U64 *mmu_find_pte(U64 addr, U32 level)
{
    U64 *pte = NULL;
    U64 idx;
    U32 i;
    
    if (level < g_mmu_ctrl.start_level) {
        return NULL;
    }
    
    pte = (U64 *)g_mmu_ctrl.tlb_addr;
    
    for (i = g_mmu_ctrl.start_level; i < MMU_LEVEL_MAX; ++i) {
        if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
            idx = (addr >> mmu_level2shift(i)) & 0x1FF;
        } else {
            idx = (addr >> mmu_level2shift(i)) & 0x1FFF;
        }
        
        pte += idx;
        
        if (i == level) {
            return pte;
        }
        
        if (mmu_get_pte_type(pte) != PTE_TYPE_TABLE) {
            return NULL;
        }
        
        if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_4K);
        } else {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_64K);
        }
    }
    
    return NULL;
}

static void mmu_set_pte_table(U64 *pte, U64 *table)
{
    *pte = PTE_TYPE_TABLE | (U64)table;
}

static S32 mmu_add_map_pte_process(mmu_mmap_region_s const *map, U64 *pte, U64 phys, U32 level)
{
    U64 *new_table = NULL;

    if (level < map->max_level) {
        if (mmu_get_pte_type(pte) == PTE_TYPE_FAULT) {
            new_table = alloc_new_table();
            if (new_table == NULL) {
                return -1;
            }
            mmu_set_pte_table(pte, new_table);
            table_usage((U64)pte, 1);
        }
    } else if (level == MMU_LEVEL_3) {
        if(mmu_get_pte_type(pte) == PTE_TYPE_FAULT)
        {
            table_usage((U64)pte, 1);
        }
        *pte = phys | map->attrs | PTE_TYPE_PAGE;
    } else {
        if(mmu_get_pte_type(pte) == PTE_TYPE_FAULT)
        {
            table_usage((U64)pte, 1);
        }
        *pte = phys | map->attrs | PTE_TYPE_BLOCK;
    }
    
    return 0;
}

static S32 mmu_release_map(mmu_mmap_region_s const *map)
{
    U64 virt = map->virt;
    U64 phys = map->phys;
    U64 max_level = map->max_level;
    U64 start_level = g_mmu_ctrl.start_level;
    U64 block_size = 0;
    U64 map_size = 0;
    U32 level;
    U64 *pte = NULL;
    U64 *ptes[MMU_LEVEL_MAX + 1];
    
    if (map->max_level <= start_level) {
        return -2;
    }
    
    while (map_size < map->size) {
        memset(ptes, 0, sizeof(ptes));
        for (level = start_level; level <= max_level; ++level) {
            block_size = 1ULL << mmu_level2shift(level);
            pte = mmu_find_pte(virt, level);
            if (pte == NULL) {
                break;
            }

            ptes[level] = pte;
        }

        for(level = max_level; level != start_level; --level) {
            pte = ptes[level];
            if(pte == NULL) {
                continue;
            }

            if(level == max_level) {
                *pte = 0;
                table_usage((U64)pte, -1);
            }

            if(table_usage((U64)pte, 0) == 1) {
                free_table((U64)pte);
                *ptes[level - 1] = 0;
                table_usage((U64)ptes[level - 1], -1);
            }
        }
        
        virt += block_size;
        phys += block_size;
        map_size += block_size;
    }
    
    return 0;
}

static S32 mmu_add_map(mmu_mmap_region_s const *map)
{
    U64 virt = map->virt;
    U64 phys = map->phys;
    U64 max_level = map->max_level;
    U64 start_level = g_mmu_ctrl.start_level;
    U64 block_size = 0;
    U64 map_size = 0;
    U32 level;
    U64 *pte = NULL;
    S32 ret;
    
    if (map->max_level <= start_level) {
        return -2;
    }
    
    while (map_size < map->size) {
        for (level = start_level; level <= max_level; ++level) {
            pte = mmu_find_pte(virt, level);
            if (pte == NULL) {
                return -3;
            }
            
            ret = mmu_add_map_pte_process(map, pte, phys, level);
            if (ret) {
                return ret;
            }
            
            if (level != start_level) {
                block_size = 1ULL << mmu_level2shift(level);
            }
        }
        
        virt += block_size;
        phys += block_size;
        map_size += block_size;
    }
    
    return 0;
}

static inline void mmu_set_ttbr_tcr_mair(U64 table, U64 tcr, U64 attr)
{
    OS_EMBED_ASM("dsb sy");
    
    OS_EMBED_ASM("msr ttbr0_el1, %0" : : "r" (table) : "memory");
    OS_EMBED_ASM("msr ttbr1_el1, %0" : : "r" (table) : "memory");
    OS_EMBED_ASM("msr tcr_el1, %0" : : "r" (tcr) : "memory");
    OS_EMBED_ASM("msr mair_el1, %0" : : "r" (attr) : "memory");
    
    OS_EMBED_ASM("isb");
}

static U32 mmu_setup_pgtables(mmu_mmap_region_s *mem_map, U32 mem_region_num, U64 tlb_addr, U64 tlb_len, U32 granule)
{
    U32 i;
    U32 ret;
    U64 tcr;
    U64 *new_table = NULL;
    
    g_mmu_ctrl.tlb_addr = tlb_addr;
    g_mmu_ctrl.tlb_size = tlb_len;
    g_mmu_ctrl.granule = granule;
    g_mmu_ctrl.start_level = 0;
    
    tcr = mmu_get_tcr(NULL, &g_mmu_ctrl.va_bits);
    
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        if (g_mmu_ctrl.va_bits < MMU_BITS_39) {
            g_mmu_ctrl.start_level = MMU_LEVEL_1;
        } else {
            g_mmu_ctrl.start_level = MMU_LEVEL_0;
        }
    } else {
        if (g_mmu_ctrl.va_bits <= MMU_BITS_36) {
            g_mmu_ctrl.start_level = MMU_LEVEL_2;
        } else {
            g_mmu_ctrl.start_level = MMU_LEVEL_1;
            return 3;
        }
    }
    
    new_table = alloc_new_table();
    if (new_table == NULL) {
        return 1;
    }
    
    for (i = 0; i < mem_region_num; ++i) {
        ret = mmu_add_map(&mem_map[i]);
        if (ret) {
            return ret;
        }
    }
    
    mmu_set_ttbr_tcr_mair(g_mmu_ctrl.tlb_addr, tcr, MEMORY_ATTRIBUTES);
    
    return 0;
}

extern mmu_mmap_region_s g_mem_map_info[];
extern size_t mmu_table_size;
static S32 mmu_setup(void)
{
    S32 ret;
    U64 page_addr;
    U64 page_len;
    
    page_addr = (U64)&g_mmu_page_begin;
    page_len = (U64)&g_mmu_page_end - (U64)&g_mmu_page_begin;

    ret = mmu_setup_pgtables(g_mem_map_info, mmu_table_size,
                             page_addr, page_len, MMU_GRANULE_4K);
    if (ret) {
        return ret;
    }
    
    return 0;
}

S32 mmu_init(void)
{
    S32 ret;

#ifdef OS_OPTION_SMP
    ret = PRT_SplLockInit(&g_mmuLock);
    if (ret) {
        return -1;
    }
#endif

    ret = mmu_setup();
    if (ret) {
        return ret;
    }

    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
    os_asm_invalidate_tlb_all();

    // 清除标志位,修复因16位对齐检查导致xen环境下异常报错(裸跑默认此位为0)
    set_sctlr((get_sctlr() | CR_C | CR_M | CR_I) & (~CR_SA));

    return 0;
}

static inline void inv_dcache_after_map_helper(void *virt, size_t size, uint32_t attrs)
{
    /*
    * DC IVAC instruction requires write access permission to the VA,
    * otherwise it can generate a permission fault
    */
    if ((attrs & PTE_BLOCK_AP_RW) != PTE_BLOCK_AP_RW) {
        return;
    }

    if (attrs & PTE_BLOCK_MEMTYPE(MT_NORMAL) == MT_NORMAL) {
        cache_invalid_by_range(virt, size);
    }
}

U64 device_map(U64 phys, U64 size, U64 level, U64 attrs)
{
    int ret;
    uintptr_t intSave;
    mmu_mmap_region_s map;

    if((phys == NULL) || (size == 0) || (level >= MMU_LEVEL_MAX))
    {
        PRT_Printf("device_map params error\n");
        return NULL;
    }

    map.virt = phys;
    map.phys = phys;
    map.size = size;
    map.max_level = level;
    map.attrs = attrs;

    intSave = PRT_SplIrqLock(&mmu_spinLock);
    ret = mmu_add_map(&map);
    if(ret != 0){
        PRT_Printf("mmu_add_map %p failed, ret = %d\n", map.phys, ret);
        PRT_SplIrqUnlock(&mmu_spinLock, intSave);
        return NULL;
    }

    os_asm_invalidate_tlb_all();
    inv_dcache_after_map_helper((void *)map.virt, map.size, map.attrs);

    PRT_SplIrqUnlock(&mmu_spinLock, intSave);

    return map.virt;
}

void device_unmap(U64 phys, U64 size, U64 level)
{
    int ret;
    uintptr_t intSave;
    mmu_mmap_region_s map;

    if((phys == NULL) || (size == 0) || (level >= MMU_LEVEL_MAX))
    {
        PRT_Printf("device_unmap params error\n");
        return;
    }

    map.virt = phys;
    map.phys = phys;
    map.size = size;
    map.max_level = level;
    map.attrs = 0;

    intSave = PRT_SplIrqLock(&mmu_spinLock);
    ret = mmu_release_map(&map);
    if(ret != 0){
        PRT_Printf("mmu_release_map %p failed, ret = %d\n", map.phys, ret);
        PRT_SplIrqUnlock(&mmu_spinLock, intSave);
        return;
    }

    os_asm_invalidate_tlb_all();
    inv_dcache_after_map_helper((void *)map.virt, map.size, map.attrs);

    PRT_SplIrqUnlock(&mmu_spinLock, intSave);

    return;
}

void print_mmu()
{
    U32 level;
    PRT_Printf("==== start show mmu =======\n");

    PRT_Printf("tlb_addr:%llx, start_level:%d\n", g_mmu_ctrl.tlb_addr, g_mmu_ctrl.start_level);

    U64* pte = (U64*)g_mmu_ctrl.tlb_addr;
    U64* pte1, * pte2;
    PRT_Printf("level:%d, addr:%llx, use time:%u\n", g_mmu_ctrl.start_level, g_mmu_ctrl.tlb_addr, table_usage((U64)pte, 0));    
    for(int i = 0; i < 512; i++){
        if(mmu_get_pte_type(pte) != PTE_TYPE_FAULT)
        {
            PRT_Printf("level:%d, addr:%llx, pte_entry:%llx\n", g_mmu_ctrl.start_level + 1, (U64)pte, (*pte)&0xFFFFFFFFFFFFF000);
            pte1 = (*pte)&0xFFFFFFFFFFFFF000;
            PRT_Printf("pte:%llx use times:%u\n", (U64)pte1, table_usage((U64)pte1, 0));
            for(int j = 0; j < 512; j++){
                if(mmu_get_pte_type(pte1) != PTE_TYPE_FAULT)
                {
                    if(mmu_get_pte_type(pte1) == PTE_TYPE_BLOCK)
                    {
                        PRT_Printf("level:%d, addr:%llx, block_entry:%llx\n", g_mmu_ctrl.start_level + 2, (U64)pte1, (*pte1)&0xFFFFFFFFFFFFF000);
                    }

                    if(mmu_get_pte_type(pte1) == PTE_TYPE_TABLE)
                    {
                        PRT_Printf("level:%d, addr:%llx, pte_entry:%llx\n", g_mmu_ctrl.start_level + 2, (U64)pte1, (*pte1)&0xFFFFFFFFFFFFF000);
                        pte2 = (*pte1)&0xFFFFFFFFFFFFF000;
                        PRT_Printf("pte:%llx use times:%u\n", (U64)pte2, table_usage((U64)pte2, 0));
                        for(int k = 0; k < 512; k++){
                            if(mmu_get_pte_type(pte2) != PTE_TYPE_FAULT)
                            {
                                PRT_Printf("level:%d, addr:%llx, pte_entry:%llx\n", g_mmu_ctrl.start_level + 3, (U64)pte2, (*pte2)&0xFFFFFFFFFFFFF000);
                            }
                            pte2 += 1;
                        }
                    }             
                }
                pte1 += 1;
                
            }
        }
        pte += 1;
    }

    PRT_Printf("==== end show mmu =======\n");
}