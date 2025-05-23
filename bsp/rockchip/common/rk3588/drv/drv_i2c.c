/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-30     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <hal_base.h>
#include "drv_i2c.h"

struct rk3588_i2c
{
    char *name;

    struct I2C_REG *reg;
    uint32_t baud_rate;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    eGPIO_bankId scl_bank;
    uint32_t scl_pin;
    ePINCTRL_configParam scl_param;

    eGPIO_bankId sda_bank;
    uint32_t sda_pin;
    ePINCTRL_configParam sda_param;

    struct rt_i2c_bus_device bus;
    struct rt_semaphore sem;
    struct I2C_HANDLE handle;
};

#define _RK3588_I2C(_name, _reg, _baud_rate, _intr_num) \
    .name = (_name),                                    \
    .reg = (_reg),                                      \
    .baud_rate = (_baud_rate),                          \
    .intr_num = (_intr_num),                            \
    .intr_prio = 8

#define _RK3588_I2C_SCL(_scl_bank, _scl_pin, _scl_param) \
    .scl_bank = (_scl_bank),                             \
    .scl_pin = (_scl_pin),                               \
    .scl_param = (_scl_param)

#define _RK3588_I2C_SDA(_sda_bank, _sda_pin, _sda_param) \
    .sda_bank = (_sda_bank),                             \
    .sda_pin = (_sda_pin),                               \
    .sda_param = (_sda_param)

static struct rk3588_i2c i2c_obj[] = {
#if defined(BSP_USING_I2C0)
    {
        _RK3588_I2C("i2c0", I2C0, BSP_I2C0_BAUDRATE, 349),
#if defined(BSP_USING_I2C0_M0)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_B3, PIN_CONFIG_MUX_FUNC2),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_A6, PIN_CONFIG_MUX_FUNC2),
#elif defined(BSP_USING_I2C0_M1)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_C6, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C0_M2)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_D1, PIN_CONFIG_MUX_FUNC3),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_D2, PIN_CONFIG_MUX_FUNC3),
#else
#error "Please define I2C0 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C1)
    {
        _RK3588_I2C("i2c1", I2C1, BSP_I2C1_BAUDRATE, 350),
#if defined(BSP_USING_I2C1_M0)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_B5, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C1_M1)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_B0, PIN_CONFIG_MUX_FUNC2),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_B1, PIN_CONFIG_MUX_FUNC2),
#elif defined(BSP_USING_I2C1_M2)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_D4, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_D5, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C1_M3)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_D4, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_D5, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C1_M4)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_D2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_D3, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C1 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C2)
    {
        _RK3588_I2C("i2c2", I2C2, BSP_I2C2_BAUDRATE, 351),
#if defined(BSP_USING_I2C2_M0)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C2_M1)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_C1, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C2_M2)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_A3, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_A2, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C2_M3)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_C4, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C2_M4)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_A1, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_A0, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C2 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C3)
    {
        _RK3588_I2C("i2c3", I2C3, BSP_I2C3_BAUDRATE, 352),
#if defined(BSP_USING_I2C3_M0)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_C1, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C3_M1)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C3_M2)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_A4, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_A5, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C3_M3)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_B2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_B3, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C3_M4)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_D0, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_D1, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C3 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C4)
    {
        _RK3588_I2C("i2c4", I2C4, BSP_I2C4_BAUDRATE, 353),
#if defined(BSP_USING_I2C4_M0)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_A6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_A5, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C4_M1)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_B5, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_B4, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C4_M2)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_C4, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C4_M3)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_A3, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_A2, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C4_M4)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_C7, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_C6, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C4 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C5)
    {
        _RK3588_I2C("i2c5", I2C5, BSP_I2C5_BAUDRATE, 354),
#if defined(BSP_USING_I2C5_M0)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_C7, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_D0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C5_M1)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C5_M2)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_A6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_A7, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C5_M3)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C5_M4)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C5 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C6)
    {
        _RK3588_I2C("i2c6", I2C6, BSP_I2C6_BAUDRATE, 355),
#if defined(BSP_USING_I2C6_M0)
        _RK3588_I2C_SCL(GPIO_BANK0, GPIO_PIN_D0, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK0, GPIO_PIN_C7, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C6_M1)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_C3, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_C2, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C6_M2)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_C3, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_C2, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C6_M3)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_B1, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_B0, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C6_M4)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_A1, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_A0, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C6 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C7)
    {
        _RK3588_I2C("i2c7", I2C7, BSP_I2C7_BAUDRATE, 356),
#if defined(BSP_USING_I2C7_M0)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_D0, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_D1, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C7_M1)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_C3, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_C4, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C7_M2)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_D2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_D3, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C7_M3)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_B2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_B3, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C7 Module."
#endif
    },
#endif

#if defined(BSP_USING_I2C8)
    {
        _RK3588_I2C("i2c8", I2C8, BSP_I2C8_BAUDRATE, 357),
#if defined(BSP_USING_I2C8_M0)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_D2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_D3, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C8_M1)
        _RK3588_I2C_SCL(GPIO_BANK2, GPIO_PIN_B0, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK2, GPIO_PIN_B1, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C8_M2)
        _RK3588_I2C_SCL(GPIO_BANK1, GPIO_PIN_D6, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK1, GPIO_PIN_D7, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C8_M3)
        _RK3588_I2C_SCL(GPIO_BANK4, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK4, GPIO_PIN_C1, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_I2C8_M4)
        _RK3588_I2C_SCL(GPIO_BANK3, GPIO_PIN_C2, PIN_CONFIG_MUX_FUNC9),
        _RK3588_I2C_SDA(GPIO_BANK3, GPIO_PIN_C3, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define I2C8 Module."
#endif
    },
#endif
};

static void rk3588_i2c_isr(uintptr_t arg)
{
    struct rk3588_i2c *i2c = (struct rk3588_i2c *)arg;

    if (HAL_I2C_IRQHandler(&i2c->handle) == HAL_OK)
    {
        rt_sem_release(&i2c->sem);
    }
}

static rt_size_t rk3588_i2c_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct rt_i2c_msg *msg;
    rt_uint32_t i;
    rt_err_t ret = 0;

    struct rk3588_i2c *i2c_bus = rt_container_of(bus, struct rk3588_i2c, bus);

    for (i = 0; i < num; i++)
    {
        uint16_t flags = HAL_I2C_M_IGNORE_NAK;

        msg = &msgs[i];

        if (msg->flags & RT_I2C_ADDR_10BIT)
        {
            flags |= HAL_I2C_M_TEN;
        }

        if (msg->flags & RT_I2C_RD)
        {
            HAL_I2C_ConfigureMode(&i2c_bus->handle, REG_CON_MOD_REGISTER_TX,
                                  HAL_I2C_REG_MRXADDR_VALID(0) | (msg->addr << 1),
                                  HAL_I2C_REG_MRXADDR_VALID(0) | msg->buf[0]);

            HAL_I2C_SetupMsg(&i2c_bus->handle, msg->addr, msg->buf, msg->len, flags | HAL_I2C_M_RD);
            HAL_I2C_Transfer(&i2c_bus->handle, I2C_IT, true);
            if (rt_sem_take(&i2c_bus->sem, i2c_bus->bus.timeout) != RT_EOK)
            {
                goto out;
            }
        }
        else
        {
            HAL_I2C_ConfigureMode(&i2c_bus->handle, REG_CON_MOD_TX, 0, 0);
            HAL_I2C_SetupMsg(&i2c_bus->handle, msg->addr, msg->buf, msg->len, flags | HAL_I2C_M_WR);
            HAL_I2C_Transfer(&i2c_bus->handle, I2C_IT, true);
            if (rt_sem_take(&i2c_bus->sem, i2c_bus->bus.timeout) != RT_EOK)
            {
                goto out;
            }
        }
    }

    ret = i;
out:
    return ret;
}

static const struct rt_i2c_bus_device_ops i2c_ops = {
    rk3588_i2c_xfer,
    RT_NULL,
    RT_NULL,
};

rt_err_t rk3588_i2c_gpio_init(struct rk3588_i2c *i2c)
{
    if (HAL_PINCTRL_SetIOMUX(i2c->scl_bank, i2c->scl_pin, i2c->scl_param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    if (HAL_PINCTRL_SetIOMUX(i2c->sda_bank, i2c->sda_pin, i2c->sda_param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3588_i2c_config_init(struct rk3588_i2c *i2c)
{
    if (rt_sem_init(&i2c->sem, i2c->name, 0, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        return -RT_ERROR;
    }

    memset(&i2c->handle, 0, sizeof(i2c->handle));

    eI2C_BusSpeed speed;
    if (i2c->baud_rate < 400000)
    {
        speed = I2C_100K;
    }
    else if (i2c->baud_rate < 1000000)
    {
        speed = I2C_400K;
    }
    else
    {
        speed = I2C_1000K;
    }

    if (HAL_I2C_Init(&i2c->handle, i2c->reg, 200000000, speed) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3588_i2c_intr_init(struct rk3588_i2c *i2c)
{
    U32 ret;

    ret = PRT_HwiSetAttr(i2c->intr_num, i2c->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(i2c->intr_num, rk3588_i2c_isr, (uintptr_t)i2c);
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
        i2c_obj[i].bus.timeout = 100;

        if (rt_i2c_bus_device_register(&i2c_obj[i].bus, i2c_obj[i].name) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_i2c_gpio_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_i2c_config_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_i2c_intr_init(&i2c_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
