#include "prt_buildef.h"
#include "prt_asm_arm_external.h"

    .global   OsResetVector
    .global   OsArmSmccSmc
    .global   mmu_init
    
    .type     mmu_init, function
    .type     start, function
    .section  .text.bspinit, "ax"
    .balign   4
    
#define HCR_EL2_FMO         (1 << 3)
#define HCR_EL2_IMO         (1 << 4)
#define HCR_EL2_AMO         (1 << 5)
#define HCR_EL2_TWI         (1 << 13)
#define HCR_EL2_TWE         (1 << 14)
#define HCR_EL2_TVM         (1 << 26)
#define HCR_EL2_TGE         (1 << 27)
#define HCR_EL2_TDZ         (1 << 28)
#define HCR_EL2_HCD         (1 << 29)
#define HCR_EL2_TRVM        (1 << 30)
#define HCR_EL2_RW          (1 << 31)

#define SPSR_DBG_MASK       (1 << 9)
#define SPSR_SERR_MASK      (1 << 8)
#define SPSR_IRQ_MASK       (1 << 7)
#define SPSR_FIQ_MASK       (1 << 6)
#define SPSR_M_AARCH64      (0 << 4)
#define SPSR_M_AARCH32      (1 << 4)
#define SPSR_M_EL1H         (5)
#define SPSR_M_EL2H         (9)

#define CNTHCTL_EL2_EL1PCEN_EN  (1 << 1)
#define CNTHCTL_EL2_EL1PCTEN_EN (1 << 0)
#define CPACR_EL1_FPEN_EN       (3 << 20)

#if defined(GUEST_OS_XEN)
_head:
    B   OsElxState
    .LONG   0
    .QUAD   0
    .QUAD   0
    .QUAD   0
    .QUAD   0
    .QUAD   0
    .QUAD   0
    .ASCII	"ARM\x64"
    .LONG   0
#endif

    .global OsElxState
    .type   OsElxState, @function
OsElxState:
    MRS    x6, CurrentEL
    MOV    x2, #0x4
    CMP    w6, w2
    
    BEQ Start
    
OsEl2Entry:
    MRS    x10, CNTHCTL_EL2
    ORR    x10, x10, #0x3
    MSR    CNTHCTL_EL2, x10
    
    MRS    x10, CNTKCTL_EL1
    ORR    x10, x10, #0x3
    MSR    CNTKCTL_EL1, x10
    
    MRS    x10, MIDR_EL1
    MRS    x1, MPIDR_EL1
    MSR    VPIDR_EL2, x10
    MSR    VMPIDR_EL2, x1
    
    MOV    x10, #0x33ff
    MSR    CPTR_EL2, x10
    MSR    HSTR_EL2, xzr
    
    MRS    x10, CPACR_EL1
    MOV    x10, #3 << 20
    MSR    CPACR_EL1, x10
    
    MOV    x10, #(HCR_EL2_RW)
    ORR    x10, x10, #(HCR_EL2_HCD)
    BIC    x10, x10, #(HCR_EL2_TVM)
    BIC    x10, x10, #(HCR_EL2_TRVM)
    BIC    x10, x10, #(HCR_EL2_TGE)
    BIC    x10, x10, #(HCR_EL2_AMO)
    BIC    x10, x10, #(HCR_EL2_IMO)
    BIC    x10, x10, #(HCR_EL2_FMO)
    BIC    x10, x10, #(HCR_EL2_TWI)
    BIC    x10, x10, #(HCR_EL2_TWE)
    
    MSR    HCR_EL2, x10
    bl 		InvalidateFlushDcaches
	dsb	 	sy
	isb
    
OsEl2SwitchToEl1:
    ADR    x0, Start
    MSR    SP_EL1, XZR
    MSR    ELR_EL2, x0
    MOV    x0, XZR
    
    LDR    x20, =(SPSR_DBG_MASK | SPSR_SERR_MASK | \
                  SPSR_IRQ_MASK | SPSR_FIQ_MASK | SPSR_M_EL1H)
    MSR    SPSR_EL2, x20
    
    TLBI   ALLE1IS
    IC     IALLU
    DSB    SY
    ISB
    ERET
    
Start:
#if defined(OS_OPTION_SMP)
    OsAsmGetCoreId  x0 // 读取核号
    LDR x4, =g_cfgPrimaryCore
    LDR w4, [x4]
    CMP w0, w4
    BNE OsSlaveCoreProcess
    
#endif
    LDR    x1, =__os_sys_sp_end
    BIC    sp, x1, #0xf

    MRS    x10, CNTKCTL_EL1
    ORR    x10, x10, #0x3
    MSR    CNTKCTL_EL1, x10

    /* enable FPU */
    MRS    x10, CPACR_EL1
    MOV    x10, #3 << 20
    MSR    CPACR_EL1, x10
    ISB

    BL     mmu_init
#if !defined(OS_OPTION_SMP)
    B      OsResetVector
#endif
    BL    OsSetValidAllCoresMask
    B     OsMasterCoreProcess

OsSlaveCoreProcess:
    OsAsmGetCoreId  x0 // 读取核号
    MOV x2, #0x1000
    mul x0, x0, x2
    LDR x1, =__os_sys_sp_end
    SUB x1, x1, x0
    BIC sp, x1, #0xf

OsSlaveStart:

    BL     mmu_init

    B      OsResetVector

OsMasterCoreProcess:
    B      OsResetVector

OsEnterReset:
    B      OsEnterReset
    
    .section .text, "ax"
    .balign 4

OsArmSmccSmc:
    smc     #0x0
    ret

/* flush or invalidate dcache, refer to rt-thread __asm_flush_dcache_all */
InvalidateFlushDcacheLevel:
    lsl    x12, x0, #1
    msr    csselr_el1, x12       /* select cache level */
    isb                          /* sync change of cssidr_el1 */
    mrs    x6, ccsidr_el1        /* read the new cssidr_el1 */
    and    x2, x6, #7            /* x2 <- log2(cache line size)-4 */
    add    x2, x2, #4            /* x2 <- log2(cache line size) */
    mov    x3, #0x3ff			 
    and    x3, x3, x6, lsr #3    /* x3 <- max number of #ways, ccsidr_el1[12:3] */
    clz    w5, w3                /* bit position of #ways */
    mov    x4, #0x7fff
    and    x4, x4, x6, lsr #13    /* x4 <- max number of #sets, ccsidr_el1[27:13] */
    /* x12 <- cache level << 1 */
    /* x2 <- line length offset */
    /* x3 <- number of cache ways - 1 */
    /* x4 <- number of cache sets - 1 */
    /* x5 <- bit position of #ways */

InvalidateFlushCacheSet:
    mov    x6, x3            /* x6 <- working copy of #ways */	

InvalidateFlushCacheWay:
    lsl    x7, x6, x5		  /* x7 = x6 << x5 */
    orr    x9, x12, x7        /* x9 = x12 | x7,  map way and level to cisw value */
    lsl    x7, x4, x2
    orr    x9, x9, x7        /* map set number to cisw value */
    tbz    w1, #0, 1f		 /* x1 = 0f, flush cache */
    dc    isw, x9			 /* invalidate dcache */
    b    2f
1:    dc    cisw, x9         /* clean & invalidate by set/way */
2:    subs    x6, x6, #1        /* decrement the way */
    b.ge    InvalidateFlushCacheWay
    subs    x4, x4, #1        /* decrement the set */
    b.ge    InvalidateFlushCacheSet

    ret

.global InvalidateFlushDcaches
InvalidateFlushDcaches:
    mov    x1, x0                /* x1 = 0 flush, x1 = 1 invalidate */
    dsb    sy                    /* barrier for full system */
    mrs    x10, clidr_el1        /* read clidr_el1 */
    lsr    x11, x10, #24
    and    x11, x11, #0x7        /* x11 <- loc bit[26:24], level of cache hierarchy */
    cbz    x11, InvalidateFlushDcacheEnd        /* if loc is 0, no cache, exit */
    mov    x15, lr           /* preserve LR */
    mov    x0, #0            /* start flush at cache level 0 */
    /* x0  <- cache level */
    /* x10 <- clidr_el1 */
    /* x11 <- loc */
    /* x15 <- return address */

InvalidateFlushCachesLoopLevel:
    lsl    x12, x0, #1         /* x12 = x0 * 2 */
    add    x12, x12, x0        /* x0 <- tripled cache level */
    lsr    x12, x10, x12       /* get x10, clidr_el1[ctype-n] to x12 */
    and    x12, x12, #7        /* x12 <- cache type */
    cmp    x12, #2             /* if not 000(no-cache), 001(i-cache only) */
    b.lt   InvalidateFlushCachesSkipLevel /* skip if no cache or icache */
    bl     InvalidateFlushDcacheLevel     /* x1 = 0 flush, 1 invalidate */

InvalidateFlushCachesSkipLevel:
    add    x0, x0, #1        /* increment cache level */
    cmp    x11, x0
    b.gt   InvalidateFlushCachesLoopLevel

    mov    x0, #0
    msr    csselr_el1, x0        /* restore csselr_el1 */
    dsb    sy
    isb
    mov    lr, x15

InvalidateFlushDcacheEnd:
	ret

