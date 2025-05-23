/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-19     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <fiopad.h>
#include <fgpio.h>
#include "drv_common.h"
#include "drv_gpio.h"

#define PIN_COUNT 96

struct pin_index
{
    rt_int16_t index;

    rt_bool_t pin_enabled;
    rt_bool_t irq_enabled;

    rt_uint32_t reg_off;
    FIOPadFunc func;

    rt_uint32_t port;
    FGpioPinIndex pin;

    HwiHandle irq_num;
    HwiPrior irq_prio;

    FGpio port_inst;
    FGpioPin pin_inst;
};

#define _E2000Q_PIN(_index, _enabled, _reg_off, _func, _port, _pin, _irq_num) \
    {                                                                         \
        .index = _index,                                                      \
        .pin_enabled = _enabled,                                              \
        .irq_enabled = false,                                                 \
        .reg_off = _reg_off,                                                  \
        .func = _func,                                                        \
        .port = _port,                                                        \
        .pin = _pin,                                                          \
        .irq_num = _irq_num,                                                  \
        .irq_prio = 4,                                                        \
    }

static struct pin_index pins[] = {
    /* GPIO0 */
    _E2000Q_PIN(0, BSP_USING_P0_0, FIOPAD_AN59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_0, 140),
    _E2000Q_PIN(1, BSP_USING_P0_1, FIOPAD_J33_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_1, 141),
    _E2000Q_PIN(2, BSP_USING_P0_2, FIOPAD_N33_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_2, 142),
    _E2000Q_PIN(3, BSP_USING_P0_3, FIOPAD_L33_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_3, 143),
    _E2000Q_PIN(4, BSP_USING_P0_4, FIOPAD_N45_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_4, 144),
    _E2000Q_PIN(5, BSP_USING_P0_5, FIOPAD_N43_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_5, 145),
    _E2000Q_PIN(6, BSP_USING_P0_6, FIOPAD_L31_REG0_OFFSET, FIOPAD_FUNC6, FGPIO0_ID, FGPIO_PIN_6, 146),
    _E2000Q_PIN(7, BSP_USING_P0_7, FIOPAD_BA57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_7, 147),
    _E2000Q_PIN(8, BSP_USING_P0_8, FIOPAD_BA59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_8, 148),
    _E2000Q_PIN(9, BSP_USING_P0_9, FIOPAD_AW57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_9, 149),
    _E2000Q_PIN(10, BSP_USING_P0_10, FIOPAD_AW59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_10, 150),
    _E2000Q_PIN(11, BSP_USING_P0_11, FIOPAD_AU55_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_11, 151),
    _E2000Q_PIN(12, BSP_USING_P0_12, FIOPAD_AN57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_12, 152),
    _E2000Q_PIN(13, BSP_USING_P0_13, FIOPAD_AL59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_13, 153),
    _E2000Q_PIN(14, BSP_USING_P0_14, FIOPAD_AJ59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_14, 154),
    _E2000Q_PIN(15, BSP_USING_P0_15, FIOPAD_AJ57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO0_ID, FGPIO_PIN_15, 155),

    /* GPIO1 */
    _E2000Q_PIN(16, BSP_USING_P1_0, FIOPAD_AG59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_0, 156),
    _E2000Q_PIN(17, BSP_USING_P1_1, FIOPAD_AG57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_1, 157),
    _E2000Q_PIN(18, BSP_USING_P1_2, FIOPAD_AE59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_2, 158),
    _E2000Q_PIN(19, BSP_USING_P1_3, FIOPAD_AC59_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_3, 159),
    _E2000Q_PIN(20, BSP_USING_P1_4, FIOPAD_AC57_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_4, 160),
    _E2000Q_PIN(21, BSP_USING_P1_5, FIOPAD_AR49_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_5, 161),
    _E2000Q_PIN(22, BSP_USING_P1_6, FIOPAD_BA55_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_6, 162),
    _E2000Q_PIN(23, BSP_USING_P1_7, FIOPAD_BA53_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_7, 163),
    _E2000Q_PIN(24, BSP_USING_P1_8, FIOPAD_E37_REG0_OFFSET, FIOPAD_FUNC6, FGPIO1_ID, FGPIO_PIN_8, 164),
    _E2000Q_PIN(25, BSP_USING_P1_9, FIOPAD_E35_REG0_OFFSET, FIOPAD_FUNC6, FGPIO1_ID, FGPIO_PIN_9, 165),
    _E2000Q_PIN(26, BSP_USING_P1_10, FIOPAD_G35_REG0_OFFSET, FIOPAD_FUNC6, FGPIO1_ID, FGPIO_PIN_10, 166),
    _E2000Q_PIN(27, BSP_USING_P1_11, FIOPAD_BA49_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_11, 167),
    _E2000Q_PIN(28, BSP_USING_P1_12, FIOPAD_AW55_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_12, 168),
    _E2000Q_PIN(29, BSP_USING_P1_13, FIOPAD_A35_REG0_OFFSET, FIOPAD_FUNC5, FGPIO1_ID, FGPIO_PIN_13, 169),
    _E2000Q_PIN(30, BSP_USING_P1_14, FIOPAD_R57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO1_ID, FGPIO_PIN_14, 170),
    _E2000Q_PIN(31, BSP_USING_P1_15, FIOPAD_R59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO1_ID, FGPIO_PIN_15, 171),

    /* GPIO2 */
    _E2000Q_PIN(32, BSP_USING_P2_0, FIOPAD_R51_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_0, 172),
    _E2000Q_PIN(33, BSP_USING_P2_1, FIOPAD_R49_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_1, 173),
    _E2000Q_PIN(34, BSP_USING_P2_2, FIOPAD_N51_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_2, 174),
    _E2000Q_PIN(35, BSP_USING_P2_3, FIOPAD_AA57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO2_ID, FGPIO_PIN_3, 175),
    _E2000Q_PIN(36, BSP_USING_P2_4, FIOPAD_AA59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO2_ID, FGPIO_PIN_4, 176),
    _E2000Q_PIN(37, BSP_USING_P2_5, FIOPAD_J55_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_5, 177),
    _E2000Q_PIN(38, BSP_USING_P2_6, FIOPAD_J45_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_6, 178),
    _E2000Q_PIN(39, BSP_USING_P2_7, FIOPAD_E47_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_7, 179),
    _E2000Q_PIN(40, BSP_USING_P2_8, FIOPAD_C39_REG0_OFFSET, FIOPAD_FUNC6, FGPIO2_ID, FGPIO_PIN_8, 180),
    _E2000Q_PIN(41, BSP_USING_P2_9, FIOPAD_C37_REG0_OFFSET, FIOPAD_FUNC6, FGPIO2_ID, FGPIO_PIN_9, 181),
    _E2000Q_PIN(42, BSP_USING_P2_10, FIOPAD_A37_REG0_OFFSET, FIOPAD_FUNC6, FGPIO2_ID, FGPIO_PIN_10, 182),
    _E2000Q_PIN(43, BSP_USING_P2_11, FIOPAD_N49_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_11, 183),
    _E2000Q_PIN(44, BSP_USING_P2_12, FIOPAD_L51_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_12, 184),
    _E2000Q_PIN(45, BSP_USING_P2_13, FIOPAD_L49_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_13, 185),
    _E2000Q_PIN(46, BSP_USING_P2_14, FIOPAD_N53_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_14, 186),
    _E2000Q_PIN(47, BSP_USING_P2_15, FIOPAD_J53_REG0_OFFSET, FIOPAD_FUNC0, FGPIO2_ID, FGPIO_PIN_15, 187),

    /* GPIO3 */
    _E2000Q_PIN(48, BSP_USING_P3_0, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_0, 188),
    _E2000Q_PIN(49, BSP_USING_P3_1, FIOPAD_C49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_1, 188),
    _E2000Q_PIN(50, BSP_USING_P3_2, FIOPAD_A51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_2, 188),
    _E2000Q_PIN(51, BSP_USING_P3_3, FIOPAD_A33_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_3, 188),
    _E2000Q_PIN(52, BSP_USING_P3_4, FIOPAD_C33_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_4, 188),
    _E2000Q_PIN(53, BSP_USING_P3_5, FIOPAD_C31_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_5, 188),
    _E2000Q_PIN(54, BSP_USING_P3_6, FIOPAD_A31_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_6, 188),
    _E2000Q_PIN(55, BSP_USING_P3_7, FIOPAD_AJ53_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_7, 188),
    _E2000Q_PIN(56, BSP_USING_P3_8, FIOPAD_AL49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_8, 188),
    _E2000Q_PIN(57, BSP_USING_P3_9, FIOPAD_AL47_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_9, 188),
    _E2000Q_PIN(58, BSP_USING_P3_10, FIOPAD_AN49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_10, 188),
    _E2000Q_PIN(59, BSP_USING_P3_11, FIOPAD_AG51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_11, 188),
    _E2000Q_PIN(60, BSP_USING_P3_12, FIOPAD_AJ51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_12, 188),
    _E2000Q_PIN(61, BSP_USING_P3_13, FIOPAD_AG49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_13, 188),
    _E2000Q_PIN(62, BSP_USING_P3_14, FIOPAD_AE55_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_14, 188),
    _E2000Q_PIN(63, BSP_USING_P3_15, FIOPAD_AE53_REG0_OFFSET, FIOPAD_FUNC6, FGPIO3_ID, FGPIO_PIN_15, 188),

    /* GPIO4 */
    _E2000Q_PIN(64, BSP_USING_P4_0, FIOPAD_AG55_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_0, 189),
    _E2000Q_PIN(65, BSP_USING_P4_1, FIOPAD_AJ49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_1, 189),
    _E2000Q_PIN(66, BSP_USING_P4_2, FIOPAD_AC55_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_2, 189),
    _E2000Q_PIN(67, BSP_USING_P4_3, FIOPAD_AC53_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_3, 189),
    _E2000Q_PIN(68, BSP_USING_P4_4, FIOPAD_AE51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_4, 189),
    _E2000Q_PIN(69, BSP_USING_P4_5, FIOPAD_W51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_5, 189),
    _E2000Q_PIN(70, BSP_USING_P4_6, FIOPAD_W55_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_6, 189),
    _E2000Q_PIN(71, BSP_USING_P4_7, FIOPAD_W53_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_7, 189),
    _E2000Q_PIN(72, BSP_USING_P4_8, FIOPAD_U55_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_8, 189),
    _E2000Q_PIN(73, BSP_USING_P4_9, FIOPAD_U53_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_9, 189),
    _E2000Q_PIN(74, BSP_USING_P4_10, FIOPAD_AE49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_10, 189),
    _E2000Q_PIN(75, BSP_USING_P4_11, FIOPAD_AC49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_11, 189),
    _E2000Q_PIN(76, BSP_USING_P4_12, FIOPAD_AE47_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_12, 189),
    _E2000Q_PIN(77, BSP_USING_P4_13, FIOPAD_AA47_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_13, 189),
    _E2000Q_PIN(78, BSP_USING_P4_14, FIOPAD_AA49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_14, 189),
    _E2000Q_PIN(79, BSP_USING_P4_15, FIOPAD_W49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO4_ID, FGPIO_PIN_15, 189),

    /* GPIO5 */
    _E2000Q_PIN(80, BSP_USING_P5_0, FIOPAD_AA51_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_0, 190),
    _E2000Q_PIN(81, BSP_USING_P5_1, FIOPAD_U49_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_1, 190),
    _E2000Q_PIN(82, BSP_USING_P5_2, FIOPAD_G59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_2, 190),
    _E2000Q_PIN(83, BSP_USING_P5_3, FIOPAD_J59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_3, 190),
    _E2000Q_PIN(84, BSP_USING_P5_4, FIOPAD_L57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_4, 190),
    _E2000Q_PIN(85, BSP_USING_P5_5, FIOPAD_C59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_5, 190),
    _E2000Q_PIN(86, BSP_USING_P5_6, FIOPAD_E59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_6, 190),
    _E2000Q_PIN(87, BSP_USING_P5_7, FIOPAD_J57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_7, 190),
    _E2000Q_PIN(88, BSP_USING_P5_8, FIOPAD_L59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_8, 190),
    _E2000Q_PIN(89, BSP_USING_P5_9, FIOPAD_N59_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_9, 190),
    _E2000Q_PIN(90, BSP_USING_P5_10, FIOPAD_C57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_10, 190),
    _E2000Q_PIN(91, BSP_USING_P5_11, FIOPAD_E57_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_11, 190),
    _E2000Q_PIN(92, BSP_USING_P5_12, FIOPAD_E31_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_12, 190),
    _E2000Q_PIN(93, BSP_USING_P5_13, FIOPAD_G31_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_13, 190),
    _E2000Q_PIN(94, BSP_USING_P5_14, FIOPAD_N41_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_14, 190),
    _E2000Q_PIN(95, BSP_USING_P5_15, FIOPAD_N39_REG0_OFFSET, FIOPAD_FUNC6, FGPIO5_ID, FGPIO_PIN_15, 190),
};

struct rt_pin_irq_hdr pin_irq_hdr_tab[PIN_COUNT];

struct pin_index *e2000q_get_pin(rt_uint8_t pin)
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

static void e2000q_pin_mode(struct rt_device *device, rt_base_t pin, rt_base_t mode)
{
    struct pin_index *index;
    index = e2000q_get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    FGpioDirection direction = FGPIO_DIR_INPUT;
    FIOPadPull pull = FIOPAD_PULL_NONE;

    switch (mode)
    {
    case PIN_MODE_OUTPUT:
        direction = FGPIO_DIR_OUTPUT;
        break;
    case PIN_MODE_OUTPUT_OD:
        direction = FGPIO_DIR_OUTPUT;
        break;
    case PIN_MODE_INPUT:
        direction = FGPIO_DIR_INPUT;
        break;
    case PIN_MODE_INPUT_PULLUP:
        direction = FGPIO_DIR_INPUT;
        pull = FIOPAD_PULL_UP;
        break;
    case PIN_MODE_INPUT_PULLDOWN:
        direction = FGPIO_DIR_INPUT;
        pull = FIOPAD_PULL_DOWN;
        break;
    default:
        break;
    }

    FIOPadSetConfig(&iopad_ctrl, index->reg_off, index->func, pull, FIOPAD_DRV4);

    FGpioConfig config = *FGpioLookupConfig(index->port);
    FGpioPinId pin_id = {index->port, FGPIO_PORT_A, index->pin};

    FGpioCfgInitialize(&index->port_inst, &config);
    FGpioPinInitialize(&index->port_inst, &index->pin_inst, pin_id);
    FGpioSetDirection(&index->pin_inst, direction);
}

static void e2000q_pin_write(struct rt_device *device, rt_base_t pin, rt_base_t value)
{
    struct pin_index *index;

    index = e2000q_get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    FGpioSetOutputValue(&index->pin_inst, (FGpioPinVal)value);
}

static int e2000q_pin_read(struct rt_device *device, rt_base_t pin)
{
    struct pin_index *index = e2000q_get_pin(pin);

    if (index == RT_NULL)
    {
        return -1;
    }

    return FGpioGetInputValue(&index->pin_inst);
}

static rt_err_t e2000q_pin_attach_irq(struct rt_device *device, rt_int32_t pin, rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    rt_int32_t hdr_index = -1;

    index = e2000q_get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_EINVAL;
    }
    hdr_index = index->index;

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

static rt_err_t e2000q_pin_detach_irq(struct rt_device *device, rt_int32_t pin)
{
    const struct pin_index *index;
    rt_int32_t hdr_index = -1;

    index = e2000q_get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_EINVAL;
    }
    hdr_index = index->index;

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

static void e2000q_pin_irq_entry(HwiArg para)
{
    struct pin_index *index = (struct pin_index *)para;

    if (index->port < FGPIO3_ID)
    {
        FGpioPinInterruptHandler(0, &index->pin_inst);
    }
    else
    {
        FGpioInterruptHandler(0, &index->port_inst);
    }
}

static void e2000q_pin_irq_callback(s32 vector, void *para)
{
    struct pin_index *index = (struct pin_index *)para;

    int hdr_index = index->index;

    if ((pin_irq_hdr_tab[hdr_index].pin != -1) && (pin_irq_hdr_tab[hdr_index].hdr != RT_NULL))
    {
        pin_irq_hdr_tab[hdr_index].hdr(pin_irq_hdr_tab[hdr_index].args);
    }
}

static int e2000q_pin_irq_init(struct pin_index *index)
{
    U32 ret;
    ret = PRT_HwiSetAttr(index->irq_num, index->irq_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiCreate(index->irq_num, (HwiProcFunc)e2000q_pin_irq_entry, (HwiArg)index);
    if (ret != OS_OK)
    {
        return ret;
    }

    PRT_HwiSetRouter(index->irq_num);
    return PRT_HwiEnable(index->irq_num);
}

static rt_err_t e2000q_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    struct pin_index *index;
    rt_int32_t hdr_index = -1;

    index = e2000q_get_pin(pin);
    if (index == RT_NULL)
    {
        return -RT_EINVAL;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        hdr_index = index->index;

        if (pin_irq_hdr_tab[hdr_index].pin == -1)
        {
            return -RT_EINVAL;
        }

        FGpioIrqType trigger_mode;

        switch (pin_irq_hdr_tab[hdr_index].mode)
        {
        case PIN_IRQ_MODE_RISING:
            trigger_mode = FGPIO_IRQ_TYPE_EDGE_RISING;
            break;
        case PIN_IRQ_MODE_FALLING:
            trigger_mode = FGPIO_IRQ_TYPE_EDGE_FALLING;
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:
            trigger_mode = FGPIO_IRQ_TYPE_LEVEL_HIGH;
            break;
        case PIN_IRQ_MODE_LOW_LEVEL:
            trigger_mode = FGPIO_IRQ_TYPE_LEVEL_LOW;
            break;
        default:
            return -RT_EINVAL;
        }

        if (index->port < FGPIO3_ID)
        {
            if (e2000q_pin_irq_init(index) != 0)
            {
                return -RT_ERROR;
            }
        }
        else
        {
            int flags = 0;
            for (int i = 0; i < 16; i++)
            {
                if (pins[16 * index->port + i].irq_enabled == RT_TRUE)
                {
                    flags = 1;
                    break;
                }
            }

            if (!flags)
            {
                index->irq_enabled = RT_TRUE;
                if (e2000q_pin_irq_init(index) != 0)
                {
                    return -RT_ERROR;
                }
            }
        }

        FGpioSetInterruptMask(&index->pin_inst, false);
        FGpioSetInterruptType(&index->pin_inst, trigger_mode);
        FGpioRegisterInterruptCB(&index->pin_inst, e2000q_pin_irq_callback, index, false);
        FGpioSetInterruptMask(&index->pin_inst, true);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        index->irq_enabled = RT_FALSE;
        if (index->port < FGPIO3_ID)
        {
            PRT_HwiDisable(index->irq_num);
        }
        else
        {
            int flags = 0;
            for (int i = 0; i < 16; i++)
            {
                if (pins[16 * index->port + i].irq_enabled == RT_TRUE)
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
    }
    else
    {
        return RT_EINVAL;
    }

    return RT_EOK;
}

static rt_base_t e2000q_pin_get(const char *name)
{
    int len = strlen(name);
    rt_base_t pin = -1;

    if ((len < 4) && (len > 5))
    {
        return -1;
    }

    if ((name[1] < '0') || (name[1] > '5') || (name[2] != '.'))
    {
        return -1;
    }

    int tmp = atoi(name + 3);
    if (tmp == 0)
    {
        if (!strcmp(name + 1, "0.0"))
        {
            return 0;
        }
    }

    if ((tmp >= 0) && (tmp <= 15))
    {
        pin = 16 * (name[1] - '0') + tmp;
    }

    return pin;
}

const static struct rt_pin_ops _e2000q_pin_ops = {
    e2000q_pin_mode,
    e2000q_pin_write,
    e2000q_pin_read,
    e2000q_pin_attach_irq,
    e2000q_pin_detach_irq,
    e2000q_pin_irq_enable,
    e2000q_pin_get,
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

    return rt_device_pin_register("pin", &_e2000q_pin_ops, RT_NULL);
}
