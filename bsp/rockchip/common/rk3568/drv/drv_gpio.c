/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-31     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <hal_base.h>
#include "drv_gpio.h"

#define PIN_COUNT 160

#define GPIO0_INT 65
#define GPIO1_INT 66
#define GPIO2_INT 67
#define GPIO3_INT 68
#define GPIO4_INT 69

struct pin_index
{
    rt_int16_t index;

    rt_bool_t pin_enabled;
    rt_bool_t irq_enabled;

    struct GPIO_REG *gpio;
    eGPIO_bankId bank;
    ePINCTRL_GPIO_PINS pin;

    HwiHandle irq_num;
    HwiPrior irq_prio;
};

#define _ROCKCHIP_PIN(_index, _enabled, _gpio, _bank, _pin, _irq_num) \
    {                                                                 \
        .index = (_index),                                            \
        .pin_enabled = (_enabled),                                    \
        .irq_enabled = false,                                         \
        .gpio = (_gpio),                                              \
        .bank = (_bank),                                              \
        .pin = (_pin),                                                \
        .irq_num = (_irq_num),                                        \
        .irq_prio = 10,                                               \
    }

/**
 * 描述：在混合部署环境中，IO资源应当提前分配好，否则会引起其他Guest OS的崩溃，将对应的引脚的pin_enabled设置为true即可
 */
static struct pin_index pins[PIN_COUNT] = {
    _ROCKCHIP_PIN(0, BSP_USING_P0_A0, GPIO0, GPIO_BANK0, GPIO_PIN_A0, GPIO0_INT),
    _ROCKCHIP_PIN(1, BSP_USING_P0_A1, GPIO0, GPIO_BANK0, GPIO_PIN_A1, GPIO0_INT),
    _ROCKCHIP_PIN(2, BSP_USING_P0_A2, GPIO0, GPIO_BANK0, GPIO_PIN_A2, GPIO0_INT),
    _ROCKCHIP_PIN(3, BSP_USING_P0_A3, GPIO0, GPIO_BANK0, GPIO_PIN_A3, GPIO0_INT),
    _ROCKCHIP_PIN(4, BSP_USING_P0_A4, GPIO0, GPIO_BANK0, GPIO_PIN_A4, GPIO0_INT),
    _ROCKCHIP_PIN(5, BSP_USING_P0_A5, GPIO0, GPIO_BANK0, GPIO_PIN_A5, GPIO0_INT),
    _ROCKCHIP_PIN(6, BSP_USING_P0_A6, GPIO0, GPIO_BANK0, GPIO_PIN_A6, GPIO0_INT),
    _ROCKCHIP_PIN(7, BSP_USING_P0_A7, GPIO0, GPIO_BANK0, GPIO_PIN_A7, GPIO0_INT),

    _ROCKCHIP_PIN(8, BSP_USING_P0_B0, GPIO0, GPIO_BANK0, GPIO_PIN_B0, GPIO0_INT),
    _ROCKCHIP_PIN(9, BSP_USING_P0_B1, GPIO0, GPIO_BANK0, GPIO_PIN_B1, GPIO0_INT),
    _ROCKCHIP_PIN(10, BSP_USING_P0_B2, GPIO0, GPIO_BANK0, GPIO_PIN_B2, GPIO0_INT),
    _ROCKCHIP_PIN(11, BSP_USING_P0_B3, GPIO0, GPIO_BANK0, GPIO_PIN_B3, GPIO0_INT),
    _ROCKCHIP_PIN(12, BSP_USING_P0_B4, GPIO0, GPIO_BANK0, GPIO_PIN_B4, GPIO0_INT),
    _ROCKCHIP_PIN(13, BSP_USING_P0_B5, GPIO0, GPIO_BANK0, GPIO_PIN_B5, GPIO0_INT),
    _ROCKCHIP_PIN(14, BSP_USING_P0_B6, GPIO0, GPIO_BANK0, GPIO_PIN_B6, GPIO0_INT),
    _ROCKCHIP_PIN(15, BSP_USING_P0_B7, GPIO0, GPIO_BANK0, GPIO_PIN_B7, GPIO0_INT),

    _ROCKCHIP_PIN(16, BSP_USING_P0_C0, GPIO0, GPIO_BANK0, GPIO_PIN_C0, GPIO0_INT),
    _ROCKCHIP_PIN(17, BSP_USING_P0_C1, GPIO0, GPIO_BANK0, GPIO_PIN_C1, GPIO0_INT),
    _ROCKCHIP_PIN(18, BSP_USING_P0_C2, GPIO0, GPIO_BANK0, GPIO_PIN_C2, GPIO0_INT),
    _ROCKCHIP_PIN(19, BSP_USING_P0_C3, GPIO0, GPIO_BANK0, GPIO_PIN_C3, GPIO0_INT),
    _ROCKCHIP_PIN(20, BSP_USING_P0_C4, GPIO0, GPIO_BANK0, GPIO_PIN_C4, GPIO0_INT),
    _ROCKCHIP_PIN(21, BSP_USING_P0_C5, GPIO0, GPIO_BANK0, GPIO_PIN_C5, GPIO0_INT),
    _ROCKCHIP_PIN(22, BSP_USING_P0_C6, GPIO0, GPIO_BANK0, GPIO_PIN_C6, GPIO0_INT),
    _ROCKCHIP_PIN(23, BSP_USING_P0_C7, GPIO0, GPIO_BANK0, GPIO_PIN_C7, GPIO0_INT),

    _ROCKCHIP_PIN(24, BSP_USING_P0_D0, GPIO0, GPIO_BANK0, GPIO_PIN_D0, GPIO0_INT),
    _ROCKCHIP_PIN(25, BSP_USING_P0_D1, GPIO0, GPIO_BANK0, GPIO_PIN_D1, GPIO0_INT),
    _ROCKCHIP_PIN(26, BSP_USING_P0_D2, GPIO0, GPIO_BANK0, GPIO_PIN_D2, GPIO0_INT),
    _ROCKCHIP_PIN(27, BSP_USING_P0_D3, GPIO0, GPIO_BANK0, GPIO_PIN_D3, GPIO0_INT),
    _ROCKCHIP_PIN(28, BSP_USING_P0_D4, GPIO0, GPIO_BANK0, GPIO_PIN_D4, GPIO0_INT),
    _ROCKCHIP_PIN(29, BSP_USING_P0_D5, GPIO0, GPIO_BANK0, GPIO_PIN_D5, GPIO0_INT),
    _ROCKCHIP_PIN(30, BSP_USING_P0_D6, GPIO0, GPIO_BANK0, GPIO_PIN_D6, GPIO0_INT),
    _ROCKCHIP_PIN(31, BSP_USING_P0_D7, GPIO0, GPIO_BANK0, GPIO_PIN_D7, GPIO0_INT),

    _ROCKCHIP_PIN(32, BSP_USING_P1_A0, GPIO1, GPIO_BANK1, GPIO_PIN_A0, GPIO1_INT),
    _ROCKCHIP_PIN(33, BSP_USING_P1_A1, GPIO1, GPIO_BANK1, GPIO_PIN_A1, GPIO1_INT),
    _ROCKCHIP_PIN(34, BSP_USING_P1_A2, GPIO1, GPIO_BANK1, GPIO_PIN_A2, GPIO1_INT),
    _ROCKCHIP_PIN(35, BSP_USING_P1_A3, GPIO1, GPIO_BANK1, GPIO_PIN_A3, GPIO1_INT),
    _ROCKCHIP_PIN(36, BSP_USING_P1_A4, GPIO1, GPIO_BANK1, GPIO_PIN_A4, GPIO1_INT),
    _ROCKCHIP_PIN(37, BSP_USING_P1_A5, GPIO1, GPIO_BANK1, GPIO_PIN_A5, GPIO1_INT),
    _ROCKCHIP_PIN(38, BSP_USING_P1_A6, GPIO1, GPIO_BANK1, GPIO_PIN_A6, GPIO1_INT),
    _ROCKCHIP_PIN(39, BSP_USING_P1_A7, GPIO1, GPIO_BANK1, GPIO_PIN_A7, GPIO1_INT),

    _ROCKCHIP_PIN(40, BSP_USING_P1_B0, GPIO1, GPIO_BANK1, GPIO_PIN_B0, GPIO1_INT),
    _ROCKCHIP_PIN(41, BSP_USING_P1_B1, GPIO1, GPIO_BANK1, GPIO_PIN_B1, GPIO1_INT),
    _ROCKCHIP_PIN(42, BSP_USING_P1_B2, GPIO1, GPIO_BANK1, GPIO_PIN_B2, GPIO1_INT),
    _ROCKCHIP_PIN(43, BSP_USING_P1_B3, GPIO1, GPIO_BANK1, GPIO_PIN_B3, GPIO1_INT),
    _ROCKCHIP_PIN(44, BSP_USING_P1_B4, GPIO1, GPIO_BANK1, GPIO_PIN_B4, GPIO1_INT),
    _ROCKCHIP_PIN(45, BSP_USING_P1_B5, GPIO1, GPIO_BANK1, GPIO_PIN_B5, GPIO1_INT),
    _ROCKCHIP_PIN(46, BSP_USING_P1_B6, GPIO1, GPIO_BANK1, GPIO_PIN_B6, GPIO1_INT),
    _ROCKCHIP_PIN(47, BSP_USING_P1_B7, GPIO1, GPIO_BANK1, GPIO_PIN_B7, GPIO1_INT),

    _ROCKCHIP_PIN(48, BSP_USING_P1_C0, GPIO1, GPIO_BANK1, GPIO_PIN_C0, GPIO1_INT),
    _ROCKCHIP_PIN(49, BSP_USING_P1_C1, GPIO1, GPIO_BANK1, GPIO_PIN_C1, GPIO1_INT),
    _ROCKCHIP_PIN(50, BSP_USING_P1_C2, GPIO1, GPIO_BANK1, GPIO_PIN_C2, GPIO1_INT),
    _ROCKCHIP_PIN(51, BSP_USING_P1_C3, GPIO1, GPIO_BANK1, GPIO_PIN_C3, GPIO1_INT),
    _ROCKCHIP_PIN(52, BSP_USING_P1_C4, GPIO1, GPIO_BANK1, GPIO_PIN_C4, GPIO1_INT),
    _ROCKCHIP_PIN(53, BSP_USING_P1_C5, GPIO1, GPIO_BANK1, GPIO_PIN_C5, GPIO1_INT),
    _ROCKCHIP_PIN(54, BSP_USING_P1_C6, GPIO1, GPIO_BANK1, GPIO_PIN_C6, GPIO1_INT),
    _ROCKCHIP_PIN(55, BSP_USING_P1_C7, GPIO1, GPIO_BANK1, GPIO_PIN_C7, GPIO1_INT),

    _ROCKCHIP_PIN(56, BSP_USING_P1_D0, GPIO1, GPIO_BANK1, GPIO_PIN_D0, GPIO1_INT),
    _ROCKCHIP_PIN(57, BSP_USING_P1_D1, GPIO1, GPIO_BANK1, GPIO_PIN_D1, GPIO1_INT),
    _ROCKCHIP_PIN(58, BSP_USING_P1_D2, GPIO1, GPIO_BANK1, GPIO_PIN_D2, GPIO1_INT),
    _ROCKCHIP_PIN(59, BSP_USING_P1_D3, GPIO1, GPIO_BANK1, GPIO_PIN_D3, GPIO1_INT),
    _ROCKCHIP_PIN(60, BSP_USING_P1_D4, GPIO1, GPIO_BANK1, GPIO_PIN_D4, GPIO1_INT),
    _ROCKCHIP_PIN(61, BSP_USING_P1_D5, GPIO1, GPIO_BANK1, GPIO_PIN_D5, GPIO1_INT),
    _ROCKCHIP_PIN(62, BSP_USING_P1_D6, GPIO1, GPIO_BANK1, GPIO_PIN_D6, GPIO1_INT),
    _ROCKCHIP_PIN(63, BSP_USING_P1_D7, GPIO1, GPIO_BANK1, GPIO_PIN_D7, GPIO1_INT),

    _ROCKCHIP_PIN(64, BSP_USING_P2_A0, GPIO2, GPIO_BANK2, GPIO_PIN_A0, GPIO2_INT),
    _ROCKCHIP_PIN(65, BSP_USING_P2_A1, GPIO2, GPIO_BANK2, GPIO_PIN_A1, GPIO2_INT),
    _ROCKCHIP_PIN(66, BSP_USING_P2_A2, GPIO2, GPIO_BANK2, GPIO_PIN_A2, GPIO2_INT),
    _ROCKCHIP_PIN(67, BSP_USING_P2_A3, GPIO2, GPIO_BANK2, GPIO_PIN_A3, GPIO2_INT),
    _ROCKCHIP_PIN(68, BSP_USING_P2_A4, GPIO2, GPIO_BANK2, GPIO_PIN_A4, GPIO2_INT),
    _ROCKCHIP_PIN(69, BSP_USING_P2_A5, GPIO2, GPIO_BANK2, GPIO_PIN_A5, GPIO2_INT),
    _ROCKCHIP_PIN(70, BSP_USING_P2_A6, GPIO2, GPIO_BANK2, GPIO_PIN_A6, GPIO2_INT),
    _ROCKCHIP_PIN(71, BSP_USING_P2_A7, GPIO2, GPIO_BANK2, GPIO_PIN_A7, GPIO2_INT),

    _ROCKCHIP_PIN(72, BSP_USING_P2_B0, GPIO2, GPIO_BANK2, GPIO_PIN_B0, GPIO2_INT),
    _ROCKCHIP_PIN(73, BSP_USING_P2_B1, GPIO2, GPIO_BANK2, GPIO_PIN_B1, GPIO2_INT),
    _ROCKCHIP_PIN(74, BSP_USING_P2_B2, GPIO2, GPIO_BANK2, GPIO_PIN_B2, GPIO2_INT),
    _ROCKCHIP_PIN(75, BSP_USING_P2_B3, GPIO2, GPIO_BANK2, GPIO_PIN_B3, GPIO2_INT),
    _ROCKCHIP_PIN(76, BSP_USING_P2_B4, GPIO2, GPIO_BANK2, GPIO_PIN_B4, GPIO2_INT),
    _ROCKCHIP_PIN(77, BSP_USING_P2_B5, GPIO2, GPIO_BANK2, GPIO_PIN_B5, GPIO2_INT),
    _ROCKCHIP_PIN(78, BSP_USING_P2_B6, GPIO2, GPIO_BANK2, GPIO_PIN_B6, GPIO2_INT),
    _ROCKCHIP_PIN(79, BSP_USING_P2_B7, GPIO2, GPIO_BANK2, GPIO_PIN_B7, GPIO2_INT),

    _ROCKCHIP_PIN(80, BSP_USING_P2_C0, GPIO2, GPIO_BANK2, GPIO_PIN_C0, GPIO2_INT),
    _ROCKCHIP_PIN(81, BSP_USING_P2_C1, GPIO2, GPIO_BANK2, GPIO_PIN_C1, GPIO2_INT),
    _ROCKCHIP_PIN(82, BSP_USING_P2_C2, GPIO2, GPIO_BANK2, GPIO_PIN_C2, GPIO2_INT),
    _ROCKCHIP_PIN(83, BSP_USING_P2_C3, GPIO2, GPIO_BANK2, GPIO_PIN_C3, GPIO2_INT),
    _ROCKCHIP_PIN(84, BSP_USING_P2_C4, GPIO2, GPIO_BANK2, GPIO_PIN_C4, GPIO2_INT),
    _ROCKCHIP_PIN(85, BSP_USING_P2_C5, GPIO2, GPIO_BANK2, GPIO_PIN_C5, GPIO2_INT),
    _ROCKCHIP_PIN(86, BSP_USING_P2_C6, GPIO2, GPIO_BANK2, GPIO_PIN_C6, GPIO2_INT),
    _ROCKCHIP_PIN(87, BSP_USING_P2_C7, GPIO2, GPIO_BANK2, GPIO_PIN_C7, GPIO2_INT),

    _ROCKCHIP_PIN(88, BSP_USING_P2_D0, GPIO2, GPIO_BANK2, GPIO_PIN_D0, GPIO2_INT),
    _ROCKCHIP_PIN(89, BSP_USING_P2_D1, GPIO2, GPIO_BANK2, GPIO_PIN_D1, GPIO2_INT),
    _ROCKCHIP_PIN(90, BSP_USING_P2_D2, GPIO2, GPIO_BANK2, GPIO_PIN_D2, GPIO2_INT),
    _ROCKCHIP_PIN(91, BSP_USING_P2_D3, GPIO2, GPIO_BANK2, GPIO_PIN_D3, GPIO2_INT),
    _ROCKCHIP_PIN(92, BSP_USING_P2_D4, GPIO2, GPIO_BANK2, GPIO_PIN_D4, GPIO2_INT),
    _ROCKCHIP_PIN(93, BSP_USING_P2_D5, GPIO2, GPIO_BANK2, GPIO_PIN_D5, GPIO2_INT),
    _ROCKCHIP_PIN(94, BSP_USING_P2_D6, GPIO2, GPIO_BANK2, GPIO_PIN_D6, GPIO2_INT),
    _ROCKCHIP_PIN(95, BSP_USING_P2_D7, GPIO2, GPIO_BANK2, GPIO_PIN_D7, GPIO2_INT),

    _ROCKCHIP_PIN(96, BSP_USING_P3_A0, GPIO3, GPIO_BANK3, GPIO_PIN_A0, GPIO3_INT),
    _ROCKCHIP_PIN(97, BSP_USING_P3_A1, GPIO3, GPIO_BANK3, GPIO_PIN_A1, GPIO3_INT),
    _ROCKCHIP_PIN(98, BSP_USING_P3_A2, GPIO3, GPIO_BANK3, GPIO_PIN_A2, GPIO3_INT),
    _ROCKCHIP_PIN(99, BSP_USING_P3_A3, GPIO3, GPIO_BANK3, GPIO_PIN_A3, GPIO3_INT),
    _ROCKCHIP_PIN(100, BSP_USING_P3_A4, GPIO3, GPIO_BANK3, GPIO_PIN_A4, GPIO3_INT),
    _ROCKCHIP_PIN(101, BSP_USING_P3_A5, GPIO3, GPIO_BANK3, GPIO_PIN_A5, GPIO3_INT),
    _ROCKCHIP_PIN(102, BSP_USING_P3_A6, GPIO3, GPIO_BANK3, GPIO_PIN_A6, GPIO3_INT),
    _ROCKCHIP_PIN(103, BSP_USING_P3_A7, GPIO3, GPIO_BANK3, GPIO_PIN_A7, GPIO3_INT),

    _ROCKCHIP_PIN(104, BSP_USING_P3_B0, GPIO3, GPIO_BANK3, GPIO_PIN_B0, GPIO3_INT),
    _ROCKCHIP_PIN(105, BSP_USING_P3_B1, GPIO3, GPIO_BANK3, GPIO_PIN_B1, GPIO3_INT),
    _ROCKCHIP_PIN(106, BSP_USING_P3_B2, GPIO3, GPIO_BANK3, GPIO_PIN_B2, GPIO3_INT),
    _ROCKCHIP_PIN(107, BSP_USING_P3_B3, GPIO3, GPIO_BANK3, GPIO_PIN_B3, GPIO3_INT),
    _ROCKCHIP_PIN(108, BSP_USING_P3_B4, GPIO3, GPIO_BANK3, GPIO_PIN_B4, GPIO3_INT),
    _ROCKCHIP_PIN(109, BSP_USING_P3_B5, GPIO3, GPIO_BANK3, GPIO_PIN_B5, GPIO3_INT),
    _ROCKCHIP_PIN(110, BSP_USING_P3_B6, GPIO3, GPIO_BANK3, GPIO_PIN_B6, GPIO3_INT),
    _ROCKCHIP_PIN(111, BSP_USING_P3_B7, GPIO3, GPIO_BANK3, GPIO_PIN_B7, GPIO3_INT),

    _ROCKCHIP_PIN(112, BSP_USING_P3_C0, GPIO3, GPIO_BANK3, GPIO_PIN_C0, GPIO3_INT),
    _ROCKCHIP_PIN(113, BSP_USING_P3_C1, GPIO3, GPIO_BANK3, GPIO_PIN_C1, GPIO3_INT),
    _ROCKCHIP_PIN(114, BSP_USING_P3_C2, GPIO3, GPIO_BANK3, GPIO_PIN_C2, GPIO3_INT),
    _ROCKCHIP_PIN(115, BSP_USING_P3_C3, GPIO3, GPIO_BANK3, GPIO_PIN_C3, GPIO3_INT),
    _ROCKCHIP_PIN(116, BSP_USING_P3_C4, GPIO3, GPIO_BANK3, GPIO_PIN_C4, GPIO3_INT),
    _ROCKCHIP_PIN(117, BSP_USING_P3_C5, GPIO3, GPIO_BANK3, GPIO_PIN_C5, GPIO3_INT),
    _ROCKCHIP_PIN(118, BSP_USING_P3_C6, GPIO3, GPIO_BANK3, GPIO_PIN_C6, GPIO3_INT),
    _ROCKCHIP_PIN(119, BSP_USING_P3_C7, GPIO3, GPIO_BANK3, GPIO_PIN_C7, GPIO3_INT),

    _ROCKCHIP_PIN(120, BSP_USING_P3_D0, GPIO3, GPIO_BANK3, GPIO_PIN_D0, GPIO3_INT),
    _ROCKCHIP_PIN(121, BSP_USING_P3_D1, GPIO3, GPIO_BANK3, GPIO_PIN_D1, GPIO3_INT),
    _ROCKCHIP_PIN(122, BSP_USING_P3_D2, GPIO3, GPIO_BANK3, GPIO_PIN_D2, GPIO3_INT),
    _ROCKCHIP_PIN(123, BSP_USING_P3_D3, GPIO3, GPIO_BANK3, GPIO_PIN_D3, GPIO3_INT),
    _ROCKCHIP_PIN(124, BSP_USING_P3_D4, GPIO3, GPIO_BANK3, GPIO_PIN_D4, GPIO3_INT),
    _ROCKCHIP_PIN(125, BSP_USING_P3_D5, GPIO3, GPIO_BANK3, GPIO_PIN_D5, GPIO3_INT),
    _ROCKCHIP_PIN(126, BSP_USING_P3_D6, GPIO3, GPIO_BANK3, GPIO_PIN_D6, GPIO3_INT),
    _ROCKCHIP_PIN(127, BSP_USING_P3_D7, GPIO3, GPIO_BANK3, GPIO_PIN_D7, GPIO3_INT),

    _ROCKCHIP_PIN(128, BSP_USING_P4_A0, GPIO4, GPIO_BANK4, GPIO_PIN_A0, GPIO4_INT),
    _ROCKCHIP_PIN(129, BSP_USING_P4_A1, GPIO4, GPIO_BANK4, GPIO_PIN_A1, GPIO4_INT),
    _ROCKCHIP_PIN(130, BSP_USING_P4_A2, GPIO4, GPIO_BANK4, GPIO_PIN_A2, GPIO4_INT),
    _ROCKCHIP_PIN(131, BSP_USING_P4_A3, GPIO4, GPIO_BANK4, GPIO_PIN_A3, GPIO4_INT),
    _ROCKCHIP_PIN(132, BSP_USING_P4_A4, GPIO4, GPIO_BANK4, GPIO_PIN_A4, GPIO4_INT),
    _ROCKCHIP_PIN(133, BSP_USING_P4_A5, GPIO4, GPIO_BANK4, GPIO_PIN_A5, GPIO4_INT),
    _ROCKCHIP_PIN(134, BSP_USING_P4_A6, GPIO4, GPIO_BANK4, GPIO_PIN_A6, GPIO4_INT),
    _ROCKCHIP_PIN(135, BSP_USING_P4_A7, GPIO4, GPIO_BANK4, GPIO_PIN_A7, GPIO4_INT),

    _ROCKCHIP_PIN(136, BSP_USING_P4_B0, GPIO4, GPIO_BANK4, GPIO_PIN_B0, GPIO4_INT),
    _ROCKCHIP_PIN(137, BSP_USING_P4_B1, GPIO4, GPIO_BANK4, GPIO_PIN_B1, GPIO4_INT),
    _ROCKCHIP_PIN(138, BSP_USING_P4_B2, GPIO4, GPIO_BANK4, GPIO_PIN_B2, GPIO4_INT),
    _ROCKCHIP_PIN(139, BSP_USING_P4_B3, GPIO4, GPIO_BANK4, GPIO_PIN_B3, GPIO4_INT),
    _ROCKCHIP_PIN(140, BSP_USING_P4_B4, GPIO4, GPIO_BANK4, GPIO_PIN_B4, GPIO4_INT),
    _ROCKCHIP_PIN(141, BSP_USING_P4_B5, GPIO4, GPIO_BANK4, GPIO_PIN_B5, GPIO4_INT),
    _ROCKCHIP_PIN(142, BSP_USING_P4_B6, GPIO4, GPIO_BANK4, GPIO_PIN_B6, GPIO4_INT),
    _ROCKCHIP_PIN(143, BSP_USING_P4_B7, GPIO4, GPIO_BANK4, GPIO_PIN_B7, GPIO4_INT),

    _ROCKCHIP_PIN(144, BSP_USING_P4_C0, GPIO4, GPIO_BANK4, GPIO_PIN_C0, GPIO4_INT),
    _ROCKCHIP_PIN(145, BSP_USING_P4_C1, GPIO4, GPIO_BANK4, GPIO_PIN_C1, GPIO4_INT),
    _ROCKCHIP_PIN(146, BSP_USING_P4_C2, GPIO4, GPIO_BANK4, GPIO_PIN_C2, GPIO4_INT),
    _ROCKCHIP_PIN(147, BSP_USING_P4_C3, GPIO4, GPIO_BANK4, GPIO_PIN_C3, GPIO4_INT),
    _ROCKCHIP_PIN(148, BSP_USING_P4_C4, GPIO4, GPIO_BANK4, GPIO_PIN_C4, GPIO4_INT),
    _ROCKCHIP_PIN(149, BSP_USING_P4_C5, GPIO4, GPIO_BANK4, GPIO_PIN_C5, GPIO4_INT),
    _ROCKCHIP_PIN(150, BSP_USING_P4_C6, GPIO4, GPIO_BANK4, GPIO_PIN_C6, GPIO4_INT),
    _ROCKCHIP_PIN(151, BSP_USING_P4_C7, GPIO4, GPIO_BANK4, GPIO_PIN_C7, GPIO4_INT),

    _ROCKCHIP_PIN(152, BSP_USING_P4_D0, GPIO4, GPIO_BANK4, GPIO_PIN_D0, GPIO4_INT),
    _ROCKCHIP_PIN(153, BSP_USING_P4_D1, GPIO4, GPIO_BANK4, GPIO_PIN_D1, GPIO4_INT),
    _ROCKCHIP_PIN(154, BSP_USING_P4_D2, GPIO4, GPIO_BANK4, GPIO_PIN_D2, GPIO4_INT),
    _ROCKCHIP_PIN(155, BSP_USING_P4_D3, GPIO4, GPIO_BANK4, GPIO_PIN_D3, GPIO4_INT),
    _ROCKCHIP_PIN(156, BSP_USING_P4_D4, GPIO4, GPIO_BANK4, GPIO_PIN_D4, GPIO4_INT),
    _ROCKCHIP_PIN(157, BSP_USING_P4_D5, GPIO4, GPIO_BANK4, GPIO_PIN_D5, GPIO4_INT),
    _ROCKCHIP_PIN(158, BSP_USING_P4_D6, GPIO4, GPIO_BANK4, GPIO_PIN_D6, GPIO4_INT),
    _ROCKCHIP_PIN(159, BSP_USING_P4_D7, GPIO4, GPIO_BANK4, GPIO_PIN_D7, GPIO4_INT),
};

struct rt_pin_irq_hdr pin_irq_hdr_tab[PIN_COUNT];

struct pin_index *rk3568_get_pin(rt_uint8_t pin)
{
    struct pin_index *index;

    if (pin < PIN_COUNT)
    {
        index = &pins[pin];
        if (index->pin_enabled == RT_FALSE)
            index = RT_NULL;
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};

static void rk3568_pin_mode(struct rt_device *device, rt_base_t pin, rt_base_t mode)
{
    struct pin_index *index = rk3568_get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    ePINCTRL_configParam param = PIN_CONFIG_MUX_FUNC0;
    eGPIO_pinDirection direction = GPIO_OUT;

    switch (mode)
    {
    case PIN_MODE_OUTPUT:
        direction = GPIO_OUT;
        param |= PIN_CONFIG_PUL_NORMAL;
        break;
    case PIN_MODE_OUTPUT_OD:
        direction = GPIO_OUT;
        param |= PIN_CONFIG_PUL_NORMAL;
        break;
    case PIN_MODE_INPUT:
        direction = GPIO_IN;
        param |= PIN_CONFIG_PUL_NORMAL;
        break;
    case PIN_MODE_INPUT_PULLUP:
        direction = GPIO_IN;
        param |= PIN_CONFIG_PUL_UP;
        break;
    case PIN_MODE_INPUT_PULLDOWN:
        direction = GPIO_IN;
        param |= PIN_CONFIG_PUL_DOWN;
        break;
    default:
        param |= PIN_CONFIG_PUL_NORMAL;
        break;
    }

    HAL_PINCTRL_SetParam(index->bank, index->pin, param);
    HAL_GPIO_SetPinDirection(index->gpio, index->pin, direction);
}

static void rk3568_pin_write(struct rt_device *device, rt_base_t pin, rt_base_t value)
{
    struct pin_index *index = rk3568_get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    HAL_GPIO_SetPinLevel(index->gpio, index->pin, (eGPIO_pinLevel)value);
}

static int rk3568_pin_read(struct rt_device *device, rt_base_t pin)
{
    int value = PIN_LOW;
    struct pin_index *index = rk3568_get_pin(pin);

    if (index == RT_NULL)
    {
        return value;
    }

    value = HAL_GPIO_GetPinLevel(index->gpio, index->pin);

    return value;
}

static rt_err_t rk3568_pin_attach_irq(struct rt_device *device, rt_int32_t pin, rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    struct pin_index *index = rk3568_get_pin(pin);

    if (index == RT_NULL)
    {
        return RT_EINVAL;
    }

    rt_int32_t hdr_index = index->index;

    if (pin_irq_hdr_tab[hdr_index].pin == pin &&
        pin_irq_hdr_tab[hdr_index].mode == mode &&
        pin_irq_hdr_tab[hdr_index].hdr == hdr &&
        pin_irq_hdr_tab[hdr_index].args == args)
    {
        return RT_EOK;
    }

    if (pin_irq_hdr_tab[hdr_index].pin != -1)
    {
        return RT_EFULL;
    }

    pin_irq_hdr_tab[hdr_index].pin = pin;
    pin_irq_hdr_tab[hdr_index].mode = mode;
    pin_irq_hdr_tab[hdr_index].hdr = hdr;
    pin_irq_hdr_tab[hdr_index].args = args;

    return RT_EOK;
}

static rt_err_t rk3568_pin_detach_irq(struct rt_device *device, rt_int32_t pin)
{
    struct pin_index *index = rk3568_get_pin(pin);

    if (index == RT_NULL)
    {
        return RT_EINVAL;
    }

    rt_int32_t hdr_index = index->index;

    if (pin_irq_hdr_tab[hdr_index].pin == -1)
    {
        return RT_EOK;
    }
    pin_irq_hdr_tab[hdr_index].pin = -1;
    pin_irq_hdr_tab[hdr_index].mode = 0;
    pin_irq_hdr_tab[hdr_index].hdr = RT_NULL;
    pin_irq_hdr_tab[hdr_index].args = RT_NULL;

    return RT_EOK;
}

void HAL_GPIO_IRQDispatch(eGPIO_bankId bank, uint32_t pin)
{
    int hdr_index = 32 * bank + pin;
    if (hdr_index < 0 || hdr_index >= PIN_COUNT)
    {
        return;
    }

    if ((pin_irq_hdr_tab[hdr_index].pin != -1) && (pin_irq_hdr_tab[hdr_index].hdr != RT_NULL))
    {
        pin_irq_hdr_tab[hdr_index].hdr(pin_irq_hdr_tab[hdr_index].args);
    }
}

static void rk3568_pin_irq_handler(uintptr_t para)
{
    switch (para)
    {
    case 0:
        HAL_GPIO_IRQHandler(GPIO0, GPIO_BANK0);
        break;
    case 1:
        HAL_GPIO_IRQHandler(GPIO1, GPIO_BANK1);
        break;
    case 2:
        HAL_GPIO_IRQHandler(GPIO2, GPIO_BANK2);
        break;
    case 3:
        HAL_GPIO_IRQHandler(GPIO3, GPIO_BANK3);
        break;
    case 4:
        HAL_GPIO_IRQHandler(GPIO4, GPIO_BANK4);
        break;
    default:
        break;
    }
}

static int rk3568_pin_irq_init(struct pin_index *index)
{
    U32 ret;
    ret = PRT_HwiSetAttr(index->irq_num, index->irq_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiCreate(index->irq_num, (HwiProcFunc)rk3568_pin_irq_handler, index->bank);
    if (ret != OS_OK)
    {
        return ret;
    }

    PRT_HwiSetRouter(index->irq_num);
    return PRT_HwiEnable(index->irq_num);
}

static rt_err_t rk3568_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    struct pin_index *index = rk3568_get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_EINVAL;
    }

    int hdr_index = -1;
    int bank = index->bank;

    if (enabled == PIN_IRQ_ENABLE)
    {
        hdr_index = index->index;
        if (hdr_index < 0 || hdr_index >= PIN_COUNT)
        {
            return RT_EINVAL;
        }

        if (pin_irq_hdr_tab[hdr_index].pin == -1)
        {
            return RT_EINVAL;
        }

        eGPIO_intType trigger_mode = GPIO_INT_TYPE_NONE;
        switch (pin_irq_hdr_tab[hdr_index].mode)
        {
        case PIN_IRQ_MODE_RISING:
            trigger_mode = GPIO_INT_TYPE_EDGE_RISING;
            break;
        case PIN_IRQ_MODE_FALLING:
            trigger_mode = GPIO_INT_TYPE_EDGE_FALLING;
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            trigger_mode = GPIO_INT_TYPE_EDGE_BOTH;
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:
            trigger_mode = GPIO_INT_TYPE_LEVEL_HIGH;
            break;
        case PIN_IRQ_MODE_LOW_LEVEL:
            trigger_mode = GPIO_INT_TYPE_LEVEL_LOW;
            break;
        default:
            return RT_EINVAL;
        }

        HAL_GPIO_SetIntType(index->gpio, index->pin, trigger_mode);
        HAL_GPIO_EnableIRQ(index->gpio, index->pin);

        int flags = 0;
        for (int i = 0; i < 32; i++)
        {
            if (pins[32 * bank + i].irq_enabled == RT_TRUE)
            {
                flags = 1;
                break;
            }
        }

        int ret;
        if (!flags)
        {
            index->irq_enabled = RT_TRUE;
            ret = rk3568_pin_irq_init(index);
        }

        if (ret)
        {
            return RT_ERROR;
        }
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        index->irq_enabled = RT_FALSE;
        int flags = 0;
        for (int i = 0; i < 32; i++)
        {
            if (pins[32 * bank + i].irq_enabled == RT_TRUE)
            {
                flags = 1;
                break;
            }
        }

        if (!flags)
        {
            PRT_HwiDisable(index->irq_num);
        }
    }
    else
    {
        return RT_EINVAL;
    }

    return RT_EOK;
}

static rt_base_t rk3568_pin_get(const char *name)
{
    if (strlen(name) != 5)
    {
        return -1;
    }

    if (name[0] != 'P')
    {
        return -1;
    }

    if ((name[1] < '0') && (name[1] > '4'))
    {
        return -1;
    }

    if (name[2] != '.')
    {
        return -1;
    }

    if ((name[3] < 'A') && (name[3] > 'D'))
    {
        return -1;
    }

    if ((name[4] < '0') && (name[4] > '7'))
    {
        return -1;
    }

    rt_base_t pin = 32 * (name[1] - '0') + 8 * (name[3] - 'A') + (name[4] - '0');
    if (rk3568_get_pin(pin) == RT_NULL)
    {
        pin = -1;
    }

    return pin;
}

const static struct rt_pin_ops _rk3568_pin_ops = {
    rk3568_pin_mode,
    rk3568_pin_write,
    rk3568_pin_read,
    rk3568_pin_attach_irq,
    rk3568_pin_detach_irq,
    rk3568_pin_irq_enable,
    rk3568_pin_get,
};

int drv_pin_init(void)
{
    for (int index = 0; index < PIN_COUNT; index++)
    {
        pin_irq_hdr_tab[index].pin = -1;
        pin_irq_hdr_tab[index].mode = 0;
        pin_irq_hdr_tab[index].hdr = RT_NULL;
        pin_irq_hdr_tab[index].args = RT_NULL;
    }

    return rt_device_pin_register("pin", &_rk3568_pin_ops, RT_NULL);
}
