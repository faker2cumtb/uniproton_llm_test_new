/**
 * @file sdrv_tmr.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver tmr driver.
 *
 * Revision History:
 * -----------------
 */

#ifndef DRV_LLD_SDRV_TMR_H
#define DRV_LLD_SDRV_TMR_H

#define SDRV_TMR_STA                            0x0
#define SDRV_TMR_STA_EN                         0x4
#define SDRV_TMR_SIG_EN                         0x8
#define SDRV_TMR_STA_SHIFT(int_id)              (int_id)
#define SDRV_TMR_STA_MASK(int_id)               (0x1 << SDRV_TMR_STA_SHIFT(int_id))
#define SDRV_TMR_STA_FIFO_OVERRUN_SHIFT(id)     (20 + (id))
#define SDRV_TMR_STA_FIFO_OVERRUN_MASK(id)      (0x1 << SDRV_TMR_STA_FIFO_OVERRUN_SHIFT(id))
#define SDRV_TMR_STA_FIFO_UNDERRUN_SHIFT(id)    (16 + (id))
#define SDRV_TMR_STA_FIFO_UNDERRUN_MASK(id)     (0x1 << SDRV_TMR_STA_FIFO_UNDERRUN_SHIFT(id))
#define SDRV_TMR_STA_CNT_OVF_SHIFT(id)          (8 + (id))
#define SDRV_TMR_STA_CNT_OVF_MASK(id)           (0x1 << SDRV_TMR_STA_CNT_OVF_SHIFT(id))
#define SDRV_TMR_STA_CMP_SHIFT(id)              (4 + (id))
#define SDRV_TMR_STA_CMP_MASK(id)               (0x1 << SDRV_TMR_STA_CMP_SHIFT(id))
#define SDRV_TMR_STA_CPT_SHIFT(id)              (id)
#define SDRV_TMR_STA_CPT_MASK(id)               (0x1 << SDRV_TMR_STA_CPT_SHIFT(id))

#define SDRV_TMR_DMA_CTRL                       0x10
#define SDRV_TMR_DMA_CTRL_WML_SHIFT(id)         (16 + (id) * 4)
#define SDRV_TMR_DMA_CTRL_WML_MASK(id)          (0xf << SDRV_TMR_DMA_CTRL_WML_SHIFT(id))
#define SDRV_TMR_DMA_CTRL_OVF_SEL_SHIFT         13
#define SDRV_TMR_DMA_CTRL_OVF_SEL_MASK          (0x1 << SDRV_TMR_DMA_CTRL_OVF_SEL_SHIFT)
#define SDRV_TMR_DMA_CTRL_CHN_SEL_SHIFT(id)     (5 + (id) * 2)
#define SDRV_TMR_DMA_CTRL_CHN_SEL_MASK(id)      (0x3 << SDRV_TMR_DMA_CTRL_CHN_SEL_SHIFT(id))
#define SDRV_TMR_DMA_CTRL_CHN_SEL_CMP           0
#define SDRV_TMR_DMA_CTRL_CHN_SEL_CPT           1
#define SDRV_TMR_DMA_CTRL_CHN_SEL_OVF           2
#define SDRV_TMR_DMA_CTRL_OVF_EN_SHIFT          4
#define SDRV_TMR_DMA_CTRL_OVF_EN_MASK           (0x1 << SDRV_TMR_DMA_CTRL_OVF_EN_SHIFT)
#define SDRV_TMR_DMA_CTRL_CHN_EN_SHIFT(id)      (id)
#define SDRV_TMR_DMA_CTRL_CHN_EN_MASK(id)       (0x1 << SDRV_TMR_DMA_CTRL_CHN_EN_SHIFT(id))

#define SDRV_TMR_SOFT_RESET                     0x18
#define SDRV_TMR_SOFT_RESET_CHN_SHIFT(id)       (16 + (id))
#define SDRV_TMR_SOFT_RESET_CHN_MASK(id)        (0x1 << SDRV_TMR_SOFT_RESET_CHN_SHIFT(id))

#define SDRV_TMR_FIFO_STA                       0x1c
#define SDRV_TMR_FIFO_STA_FULL_SHIFT(id)        ((id) * 8)
#define SDRV_TMR_FIFO_STA_FULL_MASK(id)         (0x1 << SDRV_TMR_FIFO_STA_FULL_SHIFT(id))
#define SDRV_TMR_FIFO_STA_EMPTY_SHIFT(id)       (1 + (id) * 8)
#define SDRV_TMR_FIFO_STA_EMPTY_MASK(id)        (0x1 << SDRV_TMR_FIFO_STA_EMPTY_SHIFT(id))
#define SDRV_TMR_FIFO_STA_ENTRIES_SHIFT(id)     (0x2 + (id) * 8)
#define SDRV_TMR_FIFO_STA_ENTRIES_MASK(id)      (0x1f << SDRV_TMR_FIFO_STA_ENTRIES_SHIFT(id))
#define SDRV_TMR_FIFO_STA_ERR_SHIFT(id)         (7 + (id) * 8)
#define SDRV_TMR_FIFO_STA_ERR_MASK(id)          (0x1 << SDRV_TMR_FIFO_STA_ERR_SHIFT(id))

#define SDRV_TMR_CLK_CFG                        0x20
#define SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT          0
#define SDRV_TMR_CLK_CFG_DIV_NUM_MASK           (0xffff << SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT)
#define SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT      16
#define SDRV_TMR_CLK_CFG_SRC_CLK_SEL_MASK       (0x3 << SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT)

#define SDRV_TMR_CNT_CFG                        0x24
#define SDRV_TMR_CNT_CFG_CASCADE_MODE_SHIFT     6
#define SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK      (0x1 << SDRV_TMR_CNT_CFG_CASCADE_MODE_SHIFT)
#define SDRV_TMR_CNT_CFG_RLD_SHIFT(id)          (id)
#define SDRV_TMR_CNT_CFG_RLD_MASK(id)           (0x1 << SDRV_TMR_CNT_CFG_RLD_SHIFT(id))
#define SDRV_TMR_CNT_CFG_RST_EN_SHIFT(id)       (8 + (id))
#define SDRV_TMR_CNT_CFG_RST_EN_MASK(id)        (0x1 << SDRV_TMR_CNT_CFG_RST_EN_SHIFT(id))

#define SDRV_TMR_CNT_OVF(id)                    (0x28 + (id) * 4)
#define SDRV_TMR_CNT_VAL(id)                    (0x40 + (id) * 4)
#define SDRV_TMR_CNT_INIT(id)                   (0xb0 + (id) * 4)

#define SDRV_TMR_CMP_UPT(id)                    (0x60 + (id) * 16)
#define SDRV_TMR_CMP0_VAL(id)                   (0x64 + (id) * 16)
#define SDRV_TMR_CMP1_VAL(id)                   (0x68 + (id) * 16)

#define SDRV_TMR_FIFO_VAL(id)                   (0xa0 + (id) * 4)

#define SDRV_TMR_CPT_FLT                        0xc8
#define SDRV_TMR_CPT_FLT_WID_SHIFT(id)          (4 + (id) * 8)
#define SDRV_TMR_CPT_FLT_WID_MASK(id)           (0xf << SDRV_TMR_CPT_FLT_WID_SHIFT(id))
#define SDRV_TMR_CPT_FLT_DIS_SHIFT(id)          ((id) * 8)
#define SDRV_TMR_CPT_FLT_DIS_MASK(id)           (0x1 << SDRV_TMR_CPT_FLT_DIS_SHIFT(id))

#define SDRV_TMR_SSE_CTRL                       0xcc
#define SDRV_TMR_SSE_CTRL_CMP_SEL_SHIFT(id)     (8 + (id) * 2)
#define SDRV_TMR_SSE_CTRL_CMP_SEL_MASK(id)      (0x3 << SDRV_TMR_SSE_CTRL_CMP_SEL_SHIFT(id))
#define SDRV_TMR_SSE_CTRL_CMP_EN_SHIFT(id)      (4 + (id))
#define SDRV_TMR_SSE_CTRL_CMP_EN_MASK(id)       (0x1 << SDRV_TMR_SSE_CTRL_CMP_EN_SHIFT(id))
#define SDRV_TMR_SSE_CTRL_CPT_EN_SHIFT(id)      (id)
#define SDRV_TMR_SSE_CTRL_CPT_EN_MASK(id)       (0x1 << SDRV_TMR_SSE_CTRL_CPT_EN_SHIFT(id))

#define SDRV_TMR_CPT_CFG(id)                    (0x100 + (id) * 4)
#define SDRV_TMR_CPT_CFG_TRIG_MODE_SHIFT        5
#define SDRV_TMR_CPT_CFG_TRIG_MODE_MASK         (0x3 << SDRV_TMR_CPT_CFG_TRIG_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_CNT_SEL_SHIFT          3
#define SDRV_TMR_CPT_CFG_CNT_SEL_MASK           (0x3 << SDRV_TMR_CPT_CFG_CNT_SEL_SHIFT)
#define SDRV_TMR_CPT_CFG_DUAL_MODE_SHIFT        2
#define SDRV_TMR_CPT_CFG_DUAL_MODE_MASK         (0x1 << SDRV_TMR_CPT_CFG_DUAL_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_SINGLE_MODE_SHIFT      1
#define SDRV_TMR_CPT_CFG_SINGLE_MODE_MASK       (0x1 << SDRV_TMR_CPT_CFG_SINGLE_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_EN_SHIFT               0
#define SDRV_TMR_CPT_CFG_EN_MASK                (0x1 << SDRV_TMR_CPT_CFG_EN_SHIFT)

#define SDRV_TMR_CMP_CFG(id)                    (0x110 + (id) * 4)
#define SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_SHIFT   24
#define SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_MASK    (0xff << SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_SHIFT   16
#define SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_MASK    (0xff << SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_SHIFT)
#define SDRV_TMR_CMP_CFG_FRC_LOW_SHIFT          15
#define SDRV_TMR_CMP_CFG_FRC_HIGH_SHIFT         14
#define SDRV_TMR_CMP_CFG_FRC_MASK               (0x3 << SDRV_TMR_CMP_CFG_FRC_HIGH_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_SHIFT    7
#define SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_MASK     (0x7 << SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_SHIFT    4
#define SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_MASK     (0x7 << SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_CNT_SEL_SHIFT          3
#define SDRV_TMR_CMP_CFG_CNT_SEL_MASK           (0x1 << SDRV_TMR_CMP_CFG_CNT_SEL_SHIFT)
#define SDRV_TMR_CMP_CFG_DUAL_MODE_SHIFT        2
#define SDRV_TMR_CMP_CFG_DUAL_MODE_MASK         (0x1 << SDRV_TMR_CMP_CFG_DUAL_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_SINGLE_MODE_SHIFT      1
#define SDRV_TMR_CMP_CFG_SINGLE_MODE_MASK       (0x1 << SDRV_TMR_CMP_CFG_SINGLE_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_EN_SHIFT               0
#define SDRV_TMR_CMP_CFG_EN_MASK                (0x1 << SDRV_TMR_CMP_CFG_EN_SHIFT)

#define SDRV_TMR_MAX_OVF_VAL                    0xffffffff

/* sdrv tmr id */
#define SDRV_TMR_G0                             0
#define SDRV_TMR_G1                             1
#define SDRV_TMR_CHN_A                          2
#define SDRV_TMR_CHN_B                          3
#define SDRV_TMR_CHN_C                          4
#define SDRV_TMR_CHN_D                          5
#define SDRV_TMR_MAX_NUM                        6

#define TIMER1_CHN_INTR_NUM                     225
#define TIMER1_OVF_INTR_NUM                     226
#define TIMER2_CHN_INTR_NUM                     227
#define TIMER2_OVF_INTR_NUM                     228
#define TIMER3_CHN_INTR_NUM                     229
#define TIMER3_OVF_INTR_NUM                     230
#define TIMER4_CHN_INTR_NUM                     231
#define TIMER4_OVF_INTR_NUM                     232
#define TIMER5_CHN_INTR_NUM                     233
#define TIMER5_OVF_INTR_NUM                     234
#define TIMER6_CHN_INTR_NUM                     235
#define TIMER6_OVF_INTR_NUM                     236
#define TIMER7_CHN_INTR_NUM                     237
#define TIMER7_OVF_INTR_NUM                     238
#define TIMER8_CHN_INTR_NUM                     239
#define TIMER8_OVF_INTR_NUM                     240

typedef enum {
    SDRV_TMR_HF_CLK = 0,
    SDRV_TMR_AHF_CLK,
    SDRV_TMR_LF_CLK,
    SDRV_TMR_LP_CLK,
    SDRV_TMR_ERR_CLK
} sdrv_tmr_clk_src_e;

/* timer type */
typedef enum {
    TMR_TYPE_NORMAL = 0,
    TMR_TYPE_PERIOD = 1,
    TMR_TYPE_ERR
} timer_type_e;

typedef struct sdrv_tmr_dev {
    uint32_t                base;
    int                     irq;
    int                     ovf_irq;
    sdrv_tmr_clk_src_e      clk_sel;
    uint64_t                clk_frq;
    uint32_t                clk_div;
    bool                    cascase_mode;
    uint32_t                cnt_per_us;
    uint32_t                ptp_base;
    uint32_t                id;
} sdrv_tmr_dev_t;

#endif
