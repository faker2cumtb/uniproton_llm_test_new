/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-01     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <hal_base.h>
#include "drv_spi.h"

struct rk3588_spi
{
    char *name;
    struct SPI_REG *reg;

    eGPIO_bankId bank;
    uint32_t sclk_pin;
    uint32_t mosi_pin;
    uint32_t miso_pin;
    uint32_t cs0_pin;
    uint32_t cs1_pin;
    ePINCTRL_configParam param;
    uint8_t cs_select;

    struct rt_spi_bus bus;
    struct SPI_HANDLE handle;
};

#define _RK3588_SPI(_name, _reg) \
    .name = (_name),             \
    .reg = (_reg)

#define _RK3588_SPI_PIN(_bank, _sclk_pin, _mosi_pin, _miso_pin, _cs0_pin, _cs1_pin, _param, _cs_select) \
    .bank = (_bank),                                                                                    \
    .sclk_pin = (_sclk_pin),                                                                            \
    .mosi_pin = (_mosi_pin),                                                                            \
    .miso_pin = (_miso_pin),                                                                            \
    .cs0_pin = (_cs0_pin),                                                                              \
    .cs1_pin = (_cs1_pin),                                                                              \
    .param = (_param),                                                                                  \
    .cs_select = (_cs_select)

static struct rk3588_spi spi_obj[] = {
#if defined(BSP_USING_SPI0)
    {
        _RK3588_SPI("spi0", SPI0),
#if defined(BSP_USING_SPI0_M0)
        _RK3588_SPI_PIN(GPIO_BANK0, GPIO_PIN_C6, GPIO_PIN_C0, GPIO_PIN_C7, GPIO_PIN_D1, GPIO_PIN_B7,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI0_M0_CS_INDEX),
#elif defined(BSP_USING_SPI0_M1)
        _RK3588_SPI_PIN(GPIO_BANK4, GPIO_PIN_A2, GPIO_PIN_A1, GPIO_PIN_A0, GPIO_PIN_B2, GPIO_PIN_B1,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI0_M1_CS_INDEX),
#elif defined(BSP_USING_SPI0_M2)
        _RK3588_SPI_PIN(GPIO_BANK1, GPIO_PIN_B3, GPIO_PIN_B2, GPIO_PIN_B1, GPIO_PIN_B4, GPIO_PIN_B5,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI0_M2_CS_INDEX),
#elif defined(BSP_USING_SPI0_M3)
        _RK3588_SPI_PIN(GPIO_BANK3, GPIO_PIN_D3, GPIO_PIN_D2, GPIO_PIN_D1, GPIO_PIN_D4, GPIO_PIN_D5,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI0_M3_CS_INDEX),
#else
#error "Please define SPI0 Module."
#endif
    },
#endif

#if defined(BSP_USING_SPI1)
    {
        _RK3588_SPI("spi1", SPI1),
#if defined(BSP_USING_SPI1_M0)
        _RK3588_SPI_PIN(GPIO_BANK2, GPIO_PIN_C0, GPIO_PIN_C2, GPIO_PIN_C1, GPIO_PIN_C3, GPIO_PIN_C4,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI1_M0_CS_INDEX),
#elif defined(BSP_USING_SPI1_M1)
        _RK3588_SPI_PIN(GPIO_BANK3, GPIO_PIN_C1, GPIO_PIN_B7, GPIO_PIN_C0, GPIO_PIN_C2, GPIO_PIN_C3,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI1_M1_CS_INDEX),
#elif defined(BSP_USING_SPI1_M2)
        _RK3588_SPI_PIN(GPIO_BANK1, GPIO_PIN_D2, GPIO_PIN_D1, GPIO_PIN_D0, GPIO_PIN_D3, GPIO_PIN_D5,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI1_M2_CS_INDEX),
#else
#error "Please define SPI1 Module."
#endif
    },
#endif

#if defined(BSP_USING_SPI2)
    {
        _RK3588_SPI("spi2", SPI2),
#if defined(BSP_USING_SPI2_M0)
        _RK3588_SPI_PIN(GPIO_BANK1, GPIO_PIN_A6, GPIO_PIN_A5, GPIO_PIN_A4, GPIO_PIN_A7, GPIO_PIN_B0,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI2_M0_CS_INDEX),
#elif defined(BSP_USING_SPI2_M1)
        _RK3588_SPI_PIN(GPIO_BANK4, GPIO_PIN_A6, GPIO_PIN_A5, GPIO_PIN_A4, GPIO_PIN_A7, GPIO_PIN_B0,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI2_M1_CS_INDEX),
#elif defined(BSP_USING_SPI2_M2)
        _RK3588_SPI_PIN(GPIO_BANK0, GPIO_PIN_A5, GPIO_PIN_A6, GPIO_PIN_B3, GPIO_PIN_B1, GPIO_PIN_B0,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI2_M2_CS_INDEX),
#else
#error "Please define SPI2 Module."
#endif
    },
#endif

#if defined(BSP_USING_SPI3)
    {
        _RK3588_SPI("spi3", SPI3),
#if defined(BSP_USING_SPI3_M0)
        _RK3588_SPI_PIN(GPIO_BANK4, GPIO_PIN_C6, GPIO_PIN_C5, GPIO_PIN_C4, GPIO_PIN_C2, GPIO_PIN_C3,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI3_M0_CS_INDEX),
#elif defined(BSP_USING_SPI3_M1)
        _RK3588_SPI_PIN(GPIO_BANK4, GPIO_PIN_B7, GPIO_PIN_B6, GPIO_PIN_B5, GPIO_PIN_C0, GPIO_PIN_C1,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI3_M1_CS_INDEX),
#elif defined(BSP_USING_SPI3_M2)
        _RK3588_SPI_PIN(GPIO_BANK0, GPIO_PIN_D3, GPIO_PIN_D2, GPIO_PIN_D0, GPIO_PIN_D4, GPIO_PIN_D5,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI3_M2_CS_INDEX),
#elif defined(BSP_USING_SPI3_M3)
        _RK3588_SPI_PIN(GPIO_BANK3, GPIO_PIN_D0, GPIO_PIN_C7, GPIO_PIN_C6, GPIO_PIN_C4, GPIO_PIN_C5,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI3_M3_CS_INDEX),
#else
#error "Please define SPI3 Module."
#endif
    },
#endif

#if defined(BSP_USING_SPI4)
    {
        _RK3588_SPI("spi4", SPI4),
#if defined(BSP_USING_SPI4_M0)
        _RK3588_SPI_PIN(GPIO_BANK1, GPIO_PIN_C2, GPIO_PIN_C1, GPIO_PIN_C0, GPIO_PIN_C3, GPIO_PIN_C4,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI4_M0_CS_INDEX),
#elif defined(BSP_USING_SPI4_M1)
        _RK3588_SPI_PIN(GPIO_BANK3, GPIO_PIN_A2, GPIO_PIN_A1, GPIO_PIN_A0, GPIO_PIN_A3, GPIO_PIN_A4,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI4_M1_CS_INDEX),
#elif defined(BSP_USING_SPI4_M2)
        /* Because SPI4_M2 have no cs1, so keep the same with cs0 */
        _RK3588_SPI_PIN(GPIO_BANK1, GPIO_PIN_A2, GPIO_PIN_A1, GPIO_PIN_A0, GPIO_PIN_A3, GPIO_PIN_A3,
                        PIN_CONFIG_MUX_FUNC8, BSP_USING_SPI4_M2_CS_INDEX),
#else
#error "Please define SPI4 Module."
#endif
    },
#endif
};

static rt_err_t rk3588_spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *configuration)
{
    struct rk3588_spi *spi_bus = (struct rk3588_spi *)device->bus->parent.user_data;
    struct SPI_HANDLE *handle = &spi_bus->handle;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    memset(handle, 0, sizeof(struct SPI_HANDLE));

    handle->pReg = spi_bus->reg;
    handle->maxFreq = 200000000;
    handle->dmaBurstSize = 1;
    handle->config.speed = configuration->max_hz;
    handle->config.opMode = CR0_OPM_MASTER;
    handle->config.endianMode = CR0_EM_LITTLE;
    handle->config.apbTransform = CR0_BHT_16BIT; // CR0_BHT_8BIT?
    handle->config.ssd = CR0_SSD_ONE;
    handle->config.csm = CR0_CSM_0CYCLE;

    if (configuration->data_width <= 8)
    {
        handle->config.nBytes = CR0_DATA_FRAME_SIZE_8BIT;
    }
    else if (configuration->data_width <= 16)
    {
        handle->config.nBytes = CR0_DATA_FRAME_SIZE_16BIT;
    }
    else
    {
        return RT_EIO;
    }

    switch (configuration->mode & RT_SPI_MODE_3)
    {
    case RT_SPI_MODE_0:
        handle->config.clkPolarity = CR0_POLARITY_LOW;
        handle->config.clkPhase = CR0_PHASE_1EDGE;
        break;
    case RT_SPI_MODE_1:
        handle->config.clkPolarity = CR0_POLARITY_LOW;
        handle->config.clkPhase = CR0_PHASE_2EDGE;
        break;
    case RT_SPI_MODE_2:
        handle->config.clkPolarity = CR0_POLARITY_HIGH;
        handle->config.clkPhase = CR0_PHASE_1EDGE;
        break;
    case RT_SPI_MODE_3:
        handle->config.clkPolarity = CR0_POLARITY_HIGH;
        handle->config.clkPhase = CR0_PHASE_2EDGE;
        break;
    }

    /* MSB or LSB */
    if (configuration->mode & RT_SPI_MSB)
    {
        handle->config.firstBit = CR0_FIRSTBIT_MSB;
    }
    else
    {
        handle->config.firstBit = CR0_FIRSTBIT_LSB;
    }

    HAL_SPI_Stop(handle);
    HAL_SPI_Configure(handle, NULL, NULL, 0);
    HAL_SPI_FlushFifo(handle);
    HAL_SPI_SetCS(handle, 0, false);
    HAL_SPI_SetCS(handle, 1, false);
    return RT_EOK;
};

static rt_uint32_t rk3588_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct rk3588_spi *spi_bus = (struct rk3588_spi *)device->bus->parent.user_data;
    struct SPI_HANDLE *handle = &spi_bus->handle;
    rt_uint32_t length = message->length;

    RT_ASSERT(device != NULL);
    RT_ASSERT(message != NULL);

    HAL_SPI_FlushFifo(handle);
    HAL_DelayUs(5);

    if (message->cs_take)
    {
        HAL_SPI_SetCS(handle, spi_bus->cs_select, true);
    }

    {
        const rt_uint8_t *send_buf = message->send_buf;
        rt_uint8_t *recv_buf = message->recv_buf;
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

        handle->pTxBuffer = send_buf;
        handle->pTxBufferEnd = send_buf + length;
        handle->pRxBuffer = recv_buf;
        handle->pRxBufferEnd = recv_buf + length;
        handle->len = length;
        HAL_SPI_PioTransfer(handle);

        rt_free(temp_buf);
    }

_cs_release:

    if (message->cs_release)
    {
        HAL_SPI_SetCS(handle, spi_bus->cs_select, false);
    }

    return length;
};

static struct rt_spi_ops rk3588_spi_ops = {
    rk3588_spi_configure,
    rk3588_spi_xfer,
};

rt_err_t rk3588_spi_gpio_init(struct rk3588_spi *spi)
{
    uint32_t cs_pin = spi->cs0_pin;
    if (spi->cs_select == 1)
    {
        cs_pin = spi->cs1_pin;
    }

    if (HAL_PINCTRL_SetIOMUX(spi->bank, spi->sclk_pin | spi->mosi_pin | spi->miso_pin | cs_pin, spi->param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int drv_spi_init()
{
    for (int i = 0; i < sizeof(spi_obj) / sizeof(spi_obj[0]); i++)
    {
        int result = rt_spi_bus_register(&spi_obj[i].bus, spi_obj[i].name, &rk3588_spi_ops);
        if (result != RT_EOK)
        {
            return -RT_ERROR;
        }

        spi_obj[i].bus.parent.user_data = &spi_obj[i];

        if (rk3588_spi_gpio_init(&spi_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
