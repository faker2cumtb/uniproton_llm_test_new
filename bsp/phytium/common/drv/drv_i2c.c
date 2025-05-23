/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-26     LuoYuncong   the first version
 * 2024-10-30     LuoYuncong   fix read/write bugs
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <prt_config.h>
#include <hwi_router.h>
#include <fiopad.h>
#include <fmio.h>
#include <fmio_hw.h>
#include <fi2c.h>
#include <fi2c_hw.h>
#include "drv_common.h"
#include "drv_i2c.h"

struct e2000q_i2c
{
    char *name;

    FI2c ctrl;
    FI2cConfig config;
    s32 mio_id;
    u32 baud_rate;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    u32 scl_reg_off;
    u32 sda_reg_off;
    FIOPadFunc func;

    struct rt_i2c_bus_device bus;
    struct rt_semaphore sem;
};

#define _E2000Q_I2C(_name, _mio_id, _baud_rate, _scl_reg_off, _sda_reg_off, _func, _intr_num) \
    {                                                                                         \
        .name = (_name),                                                                      \
        .mio_id = (_mio_id),                                                                  \
        .baud_rate = (_baud_rate),                                                            \
        .scl_reg_off = (_scl_reg_off),                                                        \
        .sda_reg_off = (_sda_reg_off),                                                        \
        .func = (_func),                                                                      \
        .intr_num = (_intr_num),                                                              \
        .intr_prio = 6,                                                                       \
    }

static struct e2000q_i2c i2c_obj[] = {
#if defined(BSP_USING_MIO_I2C0)
    _E2000Q_I2C("mio_i2c0", FMIO0_ID, BSP_MIO_I2C0_BAUDRATE, FIOPAD_A41_REG0_OFFSET, FIOPAD_A43_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO0_ID)),
#endif

#if defined(BSP_USING_MIO_I2C1)
    _E2000Q_I2C("mio_i2c1", FMIO1_ID, BSP_MIO_I2C1_BAUDRATE, FIOPAD_A45_REG0_OFFSET, FIOPAD_C45_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO1_ID)),
#endif

#if defined(BSP_USING_MIO_I2C2)
    _E2000Q_I2C("mio_i2c2", FMIO2_ID, BSP_MIO_I2C2_BAUDRATE, FIOPAD_A47_REG0_OFFSET, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO2_ID)),
#endif

#if defined(BSP_USING_MIO_I2C3)
    _E2000Q_I2C("mio_i2c3", FMIO3_ID, BSP_MIO_I2C3_BAUDRATE, FIOPAD_BA55_REG0_OFFSET, FIOPAD_BA53_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO3_ID)),
#endif

#if defined(BSP_USING_MIO_I2C4)
    _E2000Q_I2C("mio_i2c4", FMIO4_ID, BSP_MIO_I2C4_BAUDRATE, FIOPAD_E31_REG0_OFFSET, FIOPAD_G31_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO4_ID)),
#endif

#if defined(BSP_USING_MIO_I2C5)
    _E2000Q_I2C("mio_i2c5", FMIO5_ID, BSP_MIO_I2C5_BAUDRATE, FIOPAD_N41_REG0_OFFSET, FIOPAD_N39_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO5_ID)),
#endif

#if defined(BSP_USING_MIO_I2C6)
    _E2000Q_I2C("mio_i2c6", FMIO6_ID, BSP_MIO_I2C6_BAUDRATE, FIOPAD_AA57_REG0_OFFSET, FIOPAD_AA59_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO6_ID)),
#endif

#if defined(BSP_USING_MIO_I2C7)
    _E2000Q_I2C("mio_i2c7", FMIO7_ID, BSP_MIO_I2C7_BAUDRATE, FIOPAD_L33_REG0_OFFSET, FIOPAD_N45_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO7_ID)),
#endif

#if defined(BSP_USING_MIO_I2C8)
    _E2000Q_I2C("mio_i2c8", FMIO8_ID, BSP_MIO_I2C8_BAUDRATE, FIOPAD_AA49_REG0_OFFSET, FIOPAD_W49_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO8_ID)),
#endif

#if defined(BSP_USING_MIO_I2C9)
    _E2000Q_I2C("mio_i2c9", FMIO9_ID, BSP_MIO_I2C9_BAUDRATE, FIOPAD_AA51_REG0_OFFSET, FIOPAD_U49_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO9_ID)),
#endif

#if defined(BSP_USING_MIO_I2C10)
    _E2000Q_I2C("mio_i2c10", FMIO10_ID, BSP_MIO_I2C10_BAUDRATE, FIOPAD_C49_REG0_OFFSET, FIOPAD_A51_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO10_ID)),
#endif

#if defined(BSP_USING_MIO_I2C11)
    _E2000Q_I2C("mio_i2c11", FMIO11_ID, BSP_MIO_I2C11_BAUDRATE, FIOPAD_G59_REG0_OFFSET, FIOPAD_J59_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO11_ID)),
#endif

#if defined(BSP_USING_MIO_I2C12)
    _E2000Q_I2C("mio_i2c12", FMIO12_ID, BSP_MIO_I2C12_BAUDRATE, FIOPAD_E41_REG0_OFFSET, FIOPAD_L45_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO12_ID)),
#endif

#if defined(BSP_USING_MIO_I2C13)
    _E2000Q_I2C("mio_i2c13", FMIO13_ID, BSP_MIO_I2C13_BAUDRATE, FIOPAD_J49_REG0_OFFSET, FIOPAD_N49_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO13_ID)),
#endif

#if defined(BSP_USING_MIO_I2C14)
    _E2000Q_I2C("mio_i2c14", FMIO14_ID, BSP_MIO_I2C14_BAUDRATE, FIOPAD_L51_REG0_OFFSET, FIOPAD_L49_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO14_ID)),
#endif

#if defined(BSP_USING_MIO_I2C15)
    _E2000Q_I2C("mio_i2c15", FMIO15_ID, BSP_MIO_I2C15_BAUDRATE, FIOPAD_N53_REG0_OFFSET, FIOPAD_J53_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO15_ID)),
#endif
};

static void e2000q_i2c_intr_entry(uintptr_t arg)
{
    struct e2000q_i2c *i2c = (struct e2000q_i2c *)arg;

    FI2cMasterIntrHandler(0, &i2c->ctrl);
}

static void e2000q_i2c_xfer_isr(void *instance_p, void *arg)
{
    struct e2000q_i2c *i2c = rt_container_of(instance_p, struct e2000q_i2c, ctrl);

    rt_sem_release(&i2c->sem);
}

static int e2000q_i2c_reinit(struct e2000q_i2c *i2c)
{
    static rt_uint32_t slave_addr = RT_UINT32_MAX;
    static boolean use_7bit_addr = true;

    if ((slave_addr != i2c->config.slave_addr) || (use_7bit_addr != i2c->config.use_7bit_addr))
    {
        FI2cDeInitialize(&i2c->ctrl);
        if (FI2cCfgInitialize(&i2c->ctrl, &i2c->config) != FT_SUCCESS)
        {
            return -RT_ERROR;
        }

        FI2cMasterRegisterIntrHandler(&i2c->ctrl, FI2C_EVT_MASTER_READ_DONE, e2000q_i2c_xfer_isr);
        FI2cMasterRegisterIntrHandler(&i2c->ctrl, FI2C_EVT_MASTER_WRITE_DONE, e2000q_i2c_xfer_isr);

        rt_uint32_t intr_mask = 0;
        intr_mask &= FI2C_INTR_ALL_MASK;
        intr_mask &= ~(FI2C_INTR_RX_FULL | FI2C_INTR_TX_EMPTY);
        FI2cMasterSetupIntr(&i2c->ctrl, intr_mask);
    }

    slave_addr = i2c->config.slave_addr;
    use_7bit_addr = i2c->config.use_7bit_addr;

    return RT_EOK;
}

static rt_size_t e2000q_i2c_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct rt_i2c_msg *msg;
    rt_uint32_t i;
    rt_err_t ret = 0;

    struct e2000q_i2c *i2c = rt_container_of(bus, struct e2000q_i2c, bus);

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];

        i2c->config.slave_addr = msg->addr;

        if (msg->flags & RT_I2C_ADDR_10BIT)
        {
            i2c->config.use_7bit_addr = false;
        }
        else
        {
            i2c->config.use_7bit_addr = true;
        }

        if (e2000q_i2c_reinit(i2c) != RT_EOK)
        {
            goto _xfer_out;
        }

        if (msg->flags & RT_I2C_RD)
        {
            if (FI2cMasterReadIntr(&i2c->ctrl, msg->buf[0], 1, msg->buf, msg->len) != FT_SUCCESS)
            {
                goto _xfer_out;
            }

            /* 读数据会触发多次中断 */
            for (int k = 0; k < msg->len + 1; k++)
            {
                if (rt_sem_take(&i2c->sem, i2c->bus.timeout) != RT_EOK)
                {
                    goto _xfer_out;
                }

                ret++;
            }
        }
        else
        {
            if (FI2cMasterWriteIntr(&i2c->ctrl, msg->buf[0], 1, &msg->buf[1], (msg->len > 0) ? msg->len - 1 : 0) != FT_SUCCESS)
            {
                goto _xfer_out;
            }

            /* 写数据只触发一次中断 */
            if (rt_sem_take(&i2c->sem, i2c->bus.timeout) != RT_EOK)
            {
                goto _xfer_out;
            }
        }
    }

    ret = i;
_xfer_out:
    return ret;
}

static const struct rt_i2c_bus_device_ops i2c_ops = {
    e2000q_i2c_xfer,
    RT_NULL,
    RT_NULL,
};

rt_err_t e2000q_i2c_gpio_init(struct e2000q_i2c *i2c)
{
    FIOPadSetFunc(&iopad_ctrl, i2c->scl_reg_off, i2c->func);
    FIOPadSetFunc(&iopad_ctrl, i2c->sda_reg_off, i2c->func);

    return RT_EOK;
}

rt_err_t e2000q_i2c_config_init(struct e2000q_i2c *i2c)
{
    if (rt_sem_init(&i2c->sem, i2c->name, 0, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        return -RT_ERROR;
    }

    FMioCtrl mio_ctrl;
    mio_ctrl.config = *FMioLookupConfig(i2c->mio_id);
    FMioFuncInit(&mio_ctrl, FMIO_FUNC_SET_I2C);

    i2c->config = *FI2cLookupConfig(FI2C0_ID);
    i2c->config.base_addr = FMioFuncGetAddress(&mio_ctrl, FMIO_FUNC_SET_I2C);
    i2c->config.irq_num = FMioFuncGetIrqNum(&mio_ctrl, FMIO_FUNC_SET_I2C);
    i2c->config.slave_addr = 0;
    i2c->config.use_7bit_addr = true;
    if (i2c->baud_rate >= 400000)
    {
        i2c->config.speed_rate = FI2C_SPEED_FAST_RATE;
    }
    else
    {
        i2c->config.speed_rate = FI2C_SPEED_STANDARD_RATE;
    }

    FI2cDeInitialize(&i2c->ctrl);
    if (FI2cCfgInitialize(&i2c->ctrl, &i2c->config) != FT_SUCCESS)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t e2000q_i2c_intr_init(struct e2000q_i2c *i2c)
{
    U32 ret;

    ret = PRT_HwiSetAttr(i2c->intr_num, i2c->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(i2c->intr_num, e2000q_i2c_intr_entry, (uintptr_t)i2c);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(i2c->intr_num);
    ret = PRT_HwiEnable(i2c->intr_num);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int drv_i2c_init()
{
    for (int i = 0; i < sizeof(i2c_obj) / sizeof(i2c_obj[0]); i++)
    {
        i2c_obj[i].bus.ops = &i2c_ops;
        i2c_obj[i].bus.timeout = OS_TICK_PER_SECOND / 2;

        if (rt_i2c_bus_device_register(&i2c_obj[i].bus, i2c_obj[i].name) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_i2c_gpio_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_i2c_config_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_i2c_intr_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
