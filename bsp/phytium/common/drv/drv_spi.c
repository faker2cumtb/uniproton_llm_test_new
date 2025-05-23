/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-28     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <fiopad.h>
#include <fspim.h>
#include <fspim_hw.h>
#include "drv_common.h"
#include "drv_spi.h"

struct e2000q_spi
{
    char *name;

    FSpim ctrl;
    s32 id;

    u32 sclk_reg_off;
    u32 mosi_reg_off;
    u32 miso_reg_off;
    u32 cs_reg_off[4];
    u8 cs_select;
    FIOPadFunc func;
    FIOPadFunc cs123_func;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    struct rt_spi_bus bus;
    struct rt_semaphore sem;
};

#define _E2000Q_SPI(_name, _id, _cs_select, _sclk, _mosi, _miso, _cs0, _cs1, _cs2, _cs3, _func, _cs_func, _intr_num) \
    {                                                                                                                \
        .name = (_name),                                                                                             \
        .id = (_id),                                                                                                 \
        .cs_select = (_cs_select),                                                                                   \
        .sclk_reg_off = (_sclk),                                                                                     \
        .mosi_reg_off = (_mosi),                                                                                     \
        .miso_reg_off = (_miso),                                                                                     \
        .cs_reg_off[0] = (_cs0),                                                                                     \
        .cs_reg_off[1] = (_cs1),                                                                                     \
        .cs_reg_off[2] = (_cs2),                                                                                     \
        .cs_reg_off[3] = (_cs3),                                                                                     \
        .func = (_func),                                                                                             \
        .cs123_func = (_cs_func),                                                                                    \
        .intr_num = (_intr_num),                                                                                     \
        .intr_prio = 6,                                                                                              \
    }

/**
 * @注意：E2000Q的SPI0是双复用，因此要区分M0和M1
 */
static struct e2000q_spi spi_obj[] = {

#if defined(BSP_USING_SPI0)
#if defined(BSP_USING_SPI0_M0)
    _E2000Q_SPI("spi0", FSPI0_ID, BSP_USING_SPI0_CS_INDEX,
                FIOPAD_W55_REG0_OFFSET,
                FIOPAD_W53_REG0_OFFSET,
                FIOPAD_U55_REG0_OFFSET,
                FIOPAD_U53_REG0_OFFSET,
                FIOPAD_AE49_REG0_OFFSET,
                FIOPAD_AC49_REG0_OFFSET,
                FIOPAD_AE47_REG0_OFFSET,
                FIOPAD_FUNC2, FIOPAD_FUNC2, FSPI0_IRQ_NUM),
#else
    _E2000Q_SPI("spi0", FSPI0_ID, BSP_USING_SPI0_CS_INDEX,
                FIOPAD_J35_REG0_OFFSET,
                FIOPAD_L37_REG0_OFFSET,
                FIOPAD_N35_REG0_OFFSET,
                FIOPAD_G35_REG0_OFFSET,
                FIOPAD_E35_REG0_OFFSET,
                FIOPAD_E37_REG0_OFFSET,
                FIOPAD_L39_REG0_OFFSET,
                FIOPAD_FUNC3, FIOPAD_FUNC3, FSPI0_IRQ_NUM),
#endif
#endif

#if defined(BSP_USING_SPI1)
    _E2000Q_SPI("spi1", FSPI1_ID, BSP_USING_SPI1_CS_INDEX,
                FIOPAD_N43_REG0_OFFSET,
                FIOPAD_L31_REG0_OFFSET,
                FIOPAD_J31_REG0_OFFSET,
                FIOPAD_J29_REG0_OFFSET,
                FIOPAD_N27_REG0_OFFSET,
                FIOPAD_L29_REG0_OFFSET,
                FIOPAD_N45_REG0_OFFSET,
                FIOPAD_FUNC4, FIOPAD_FUNC4, FSPI1_IRQ_NUM),
#endif

#if defined(BSP_USING_SPI2)
    _E2000Q_SPI("spi2", FSPI2_ID, BSP_USING_SPI2_CS_INDEX,
                FIOPAD_A33_REG0_OFFSET,
                FIOPAD_C33_REG0_OFFSET,
                FIOPAD_C31_REG0_OFFSET,
                FIOPAD_A31_REG0_OFFSET,
                FIOPAD_A51_REG0_OFFSET,
                FIOPAD_C49_REG0_OFFSET,
                FIOPAD_A49_REG0_OFFSET,
                FIOPAD_FUNC0, FIOPAD_FUNC4, FSPI2_IRQ_NUM),
#endif

#if defined(BSP_USING_SPI3)
    _E2000Q_SPI("spi3", FSPI3_ID, BSP_USING_SPI3_CS_INDEX,
                FIOPAD_E31_REG0_OFFSET,
                FIOPAD_G31_REG0_OFFSET,
                FIOPAD_N41_REG0_OFFSET,
                FIOPAD_N39_REG0_OFFSET,
                FIOPAD_J33_REG0_OFFSET,
                FIOPAD_N33_REG0_OFFSET,
                FIOPAD_L33_REG0_OFFSET,
                FIOPAD_FUNC4, FIOPAD_FUNC4, FSPI3_IRQ_NUM),
#endif

};

static void e2000q_spi_intr_entry(uintptr_t arg)
{
    struct e2000q_spi *spi = (struct e2000q_spi *)arg;

    FSpimInterruptHandler(0, &spi->ctrl);
}

static void e2000q_spi_xfer_isr(void *instance_p, void *arg)
{
    struct e2000q_spi *spi = (struct e2000q_spi *)arg;
    rt_sem_release(&spi->sem);
}

static rt_err_t e2000q_spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *cfg)
{
    struct e2000q_spi *spi = (struct e2000q_spi *)device->bus->parent.user_data;
    FSpimConfig config = *FSpimLookupConfig(spi->id);

    config.max_freq_hz = cfg->max_hz;
    config.slave_dev_id = spi->cs_select;

    if (cfg->mode & RT_SPI_SLAVE)
    {
        return -RT_EIO;
    }

    switch (cfg->mode & RT_SPI_MODE_3)
    {
    case RT_SPI_MODE_0:
        config.cpol = FSPIM_CPOL_LOW;
        config.cpha = FSPIM_CPHA_1_EDGE;
        break;
    case RT_SPI_MODE_1:
        config.cpol = FSPIM_CPOL_LOW;
        config.cpha = FSPIM_CPHA_2_EDGE;
        break;
    case RT_SPI_MODE_2:
        config.cpol = FSPIM_CPOL_HIGH;
        config.cpha = FSPIM_CPHA_1_EDGE;
        break;
    case RT_SPI_MODE_3:
        config.cpol = FSPIM_CPOL_HIGH;
        config.cpha = FSPIM_CPHA_2_EDGE;
        break;
    }

    if (cfg->data_width <= 8)
    {
        config.n_bytes = FSPIM_1_BYTE;
    }
    else if (cfg->data_width <= 16)
    {
        config.n_bytes = FSPIM_2_BYTE;
    }
    else
    {
        return -RT_EIO;
    }

    if (FSpimCfgInitialize(&spi->ctrl, &config) != FSPIM_SUCCESS)
    {
        return -RT_ERROR;
    }

    FSpimRegisterIntrruptHandler(&spi->ctrl, FSPIM_INTR_EVT_RX_DONE, e2000q_spi_xfer_isr, (void *)spi);

    return RT_EOK;
};

static rt_uint32_t e2000q_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *msg)
{
    struct e2000q_spi *spi = (struct e2000q_spi *)device->bus->parent.user_data;
    rt_uint32_t length = msg->length;

    if (msg->cs_take)
    {
        FSpimSetChipSelection(&spi->ctrl, TRUE);
    }

    {
        const rt_uint8_t *send_buf = msg->send_buf;
        rt_uint8_t *recv_buf = msg->recv_buf;
        rt_uint8_t *temp_buf = rt_malloc(length);

        if (temp_buf == RT_NULL)
        {
            length = 0;
            goto _cs_release;
        }

        if (send_buf == RT_NULL)
        {
            send_buf = temp_buf;
        }

        if (recv_buf == RT_NULL)
        {
            recv_buf = temp_buf;
        }

        if (FSpimTransferByInterrupt(&spi->ctrl, send_buf, recv_buf, length) != FSPIM_SUCCESS)
        {
            length = 0;
        }

        if (rt_sem_take(&spi->sem, 100) != RT_EOK)
        {
            length = 0;
        }

        rt_free(temp_buf);
    }

_cs_release:

    if (msg->cs_release)
    {
        FSpimSetChipSelection(&spi->ctrl, FALSE);
    }

    return length;
};

static struct rt_spi_ops e2000q_spi_ops = {
    e2000q_spi_configure,
    e2000q_spi_xfer,
};

rt_err_t e2000q_spi_gpio_init(struct e2000q_spi *spi)
{
    if (spi->cs_select >= 4)
    {
        return -RT_ERROR;
    }

    FIOPadSetFunc(&iopad_ctrl, spi->sclk_reg_off, spi->func);
    FIOPadSetFunc(&iopad_ctrl, spi->mosi_reg_off, spi->func);
    FIOPadSetFunc(&iopad_ctrl, spi->miso_reg_off, spi->func);

    if (spi->cs_select == 0)
    {
        FIOPadSetFunc(&iopad_ctrl, spi->cs_reg_off[0], spi->func);
    }
    else
    {
        FIOPadSetFunc(&iopad_ctrl, spi->cs_reg_off[spi->cs_select], spi->cs123_func);
    }

    return RT_EOK;
}

rt_err_t e2000q_spi_config_init(struct e2000q_spi *spi)
{
    if (rt_sem_init(&spi->sem, spi->name, 0, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t e2000q_spi_intr_init(struct e2000q_spi *spi)
{
    if (PRT_HwiSetAttr(spi->intr_num, spi->intr_prio, OS_HWI_MODE_ENGROSS) != OS_OK)
    {
        return -RT_ERROR;
    }

    if (PRT_HwiCreate(spi->intr_num, e2000q_spi_intr_entry, (uintptr_t)spi) != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(spi->intr_num);

    if (PRT_HwiEnable(spi->intr_num) != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int drv_spi_init()
{
    for (int i = 0; i < sizeof(spi_obj) / sizeof(spi_obj[0]); i++)
    {
        int result = rt_spi_bus_register(&spi_obj[i].bus, spi_obj[i].name, &e2000q_spi_ops);
        if (result != RT_EOK)
        {
            return -RT_ERROR;
        }

        spi_obj[i].bus.parent.user_data = &spi_obj[i];

        if (e2000q_spi_gpio_init(&spi_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_spi_config_init(&spi_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_spi_intr_init(&spi_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
