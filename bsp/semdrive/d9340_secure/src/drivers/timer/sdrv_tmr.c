#include "prt_sys.h"
#include "prt_task.h"
#include "sdrv_tmr.h"
#include "cpu_config.h"
#include "prt_config.h"

#define BIT_SET(x, bit) (((x) & (1UL << (bit))) ? 1 : 0)

/**
 * @brief sdrv tmr read reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] return reg val
 */
static inline uint32_t sdrv_tmr_lld_readl(uint32_t base, uint32_t offset)
{
    return REG_READ(base + offset);
}

/**
 * @brief sdrv tmr write reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] val reg val
 */
static inline void sdrv_tmr_lld_writel(uint32_t base, uint32_t offset, uint32_t val)
{
    REG_WRITE((base + offset), val);
}

/**
 * @brief sdrv tmr set ovf cnt.
 *
 * @param[in] tmr dev base
 * @param[in] tmr_id tmr id
 * @param[in] time_out ovf cnt value
 */
static void sdrv_tmr_lld_set_ovf_cnt(uint32_t base, U8 id, uint32_t ovf_cnt)
{
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_OVF(id), ovf_cnt);
}

/**
 * @brief sdrv tmr start.
 *
 * @param[in] tmr dev base
 * @param[in] tmr_id tmr id
 */
static void sdrv_tmr_lld_start_cnt(uint32_t base, U8 id)
{
    uint32_t cnt_cfg;

    cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
    cnt_cfg |= SDRV_TMR_CNT_CFG_RLD_MASK(id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);
}

/**
 * @brief sdrv tmr int enable.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_enable(uint32_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_tmr_lld_readl(base, SDRV_TMR_STA_EN);
    sta_en |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA_EN, sta_en);

    sig_en = sdrv_tmr_lld_readl(base, SDRV_TMR_SIG_EN);
    sig_en |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_SIG_EN, sig_en);
}

void sdrv_tmr_lld_set_cascase_mode(uint32_t base, bool cascase_mode)
{
    uint32_t cnt_cfg;

    /* set cascase mode */
    if (cascase_mode) {
        cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
        cnt_cfg |= SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK;
        sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);

        /* start G0 and G1 */
        sdrv_tmr_lld_set_ovf_cnt(base, SDRV_TMR_G0, SDRV_TMR_MAX_OVF_VAL);
        sdrv_tmr_lld_set_ovf_cnt(base, SDRV_TMR_G1, SDRV_TMR_MAX_OVF_VAL);
        sdrv_tmr_lld_start_cnt(base, SDRV_TMR_G0);
        sdrv_tmr_lld_start_cnt(base, SDRV_TMR_G1);
    }
    else {
        cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
        cnt_cfg &= ~(SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK);
        sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);
    }
}

/**
 * @brief sdrv tmr set timer start.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 * @param[in] count time_out time out value(ms)
 */
void sdrv_tmr_lld_timer_start(uint32_t base, U8 id, uint32_t count)
{
    sdrv_tmr_lld_set_ovf_cnt(base, id, count);
    sdrv_tmr_lld_start_cnt(base, id);
    sdrv_tmr_lld_int_enable(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
}

void sdrv_tmr_lld_init(uint32_t base, uint32_t clk, uint32_t div)
{
    uint32_t clk_cfg;

    /* set clk */
    clk_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CLK_CFG);
    clk_cfg &= ~(SDRV_TMR_CLK_CFG_DIV_NUM_MASK | SDRV_TMR_CLK_CFG_SRC_CLK_SEL_MASK);
    clk_cfg |= ((clk << SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT) |
                (div << SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT));
    sdrv_tmr_lld_writel(base, SDRV_TMR_CLK_CFG, clk_cfg);
}

/**
 * @brief sdrv tmr int disable.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_disable(uint32_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_tmr_lld_readl(base, SDRV_TMR_STA_EN);
    sta_en &= ~(SDRV_TMR_STA_MASK(int_id));
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA_EN, sta_en);

    sig_en = sdrv_tmr_lld_readl(base, SDRV_TMR_SIG_EN);
    sig_en &= ~(SDRV_TMR_STA_MASK(int_id));
    sdrv_tmr_lld_writel(base, SDRV_TMR_SIG_EN, sig_en);
}

/**
 * @brief sdrv tmr int status clear.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_clear(uint32_t base, uint32_t int_id)
{
    uint32_t sta_val;

    sta_val = sdrv_tmr_lld_readl(base, SDRV_TMR_STA);
    sta_val |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA, sta_val);
}

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 */
static void sdrv_tmr_lld_timer_stop(uint32_t base, U8 id)
{
    sdrv_tmr_lld_int_disable(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
    sdrv_tmr_lld_int_clear(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
    sdrv_tmr_lld_set_ovf_cnt(base, id, SDRV_TMR_MAX_OVF_VAL);
}


/**
 * @brief sdrv tmr get glb cnt value.
 *
 * @param[in] dev tmr dev
 * @return glb cnt count(count)
 */
uint64_t sdrv_tmr_lld_get_glb_cnt(uint32_t base)
{
    uint32_t g0_cnt, g0_cnt_pre, g1_cnt;

    g0_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G0));
    do {
        g0_cnt_pre = g0_cnt;
        g1_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G1));
        g0_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G0));
    } while (g0_cnt_pre > g0_cnt);

    return (((uint64_t)g1_cnt) << 32) + g0_cnt;
}

/**
 * @brief sdrv tmr get ovf status and clear.
 *
 * @param[in] tmr dev base
 * @return ovf status bitmap
 */
uint32_t sdrv_tmr_lld_get_int_status(uint32_t base)
{
    uint32_t sta;

    sta = sdrv_tmr_lld_readl(base, SDRV_TMR_STA);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA, sta);
    return sta;
}

