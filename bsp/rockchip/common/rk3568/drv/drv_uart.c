/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-02     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hal_base.h>
#include <hwi_router.h>
#include "drv_uart.h"

struct rk3568_uart
{
    char *name;

    struct UART_REG *reg;
    eCLOCK_Name clk;
    uint32_t baud_rate;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    HAL_Status (*io_func)(eIOFUNC_SEL mode);
    eIOFUNC_SEL io_mode;

    eGPIO_bankId rx_bank;
    uint32_t rx_pin;
    ePINCTRL_configParam rx_param;

    eGPIO_bankId tx_bank;
    uint32_t tx_pin;
    ePINCTRL_configParam tx_param;

    struct rt_serial_device serial;
};

#define _RK3568_UART(_name, _reg, _clk, _baud_rate, _intr_num) \
    .name = (_name),                                           \
    .reg = (_reg),                                             \
    .clk = (_clk),                                             \
    .baud_rate = (_baud_rate),                                 \
    .intr_num = (_intr_num),                                   \
    .intr_prio = 6

#define _RK3568_UART_FUNC(_io_func, _io_mode) \
    .io_func = (_io_func),                    \
    .io_mode = (_io_mode)

#define _RK3568_UART_RX(_rx_bank, _rx_pin, _rx_param) \
    .rx_bank = (_rx_bank),                            \
    .rx_pin = (_rx_pin),                              \
    .rx_param = (_rx_param)

#define _RK3568_UART_TX(_tx_bank, _tx_pin, _tx_param) \
    .tx_bank = (_tx_bank),                            \
    .tx_pin = (_tx_pin),                              \
    .tx_param = (_tx_param)

static struct rk3568_uart uart_obj[] = {

#if defined(BSP_USING_UART1)
    {
        _RK3568_UART("uart1", UART1, CLK_UART1, BSP_UART1_BAUDRATE, 149),
#if defined(BSP_USING_UART1_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART1, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_B3, PIN_CONFIG_MUX_FUNC2),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_B4, PIN_CONFIG_MUX_FUNC2),
#elif defined(BSP_USING_UART1_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART1, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_D7, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK3, GPIO_PIN_D6, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART1 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART2)
    {
        _RK3568_UART("uart2", UART2, CLK_UART2, BSP_UART2_BAUDRATE, 150),
#if defined(BSP_USING_UART2_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART2, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK0, GPIO_PIN_D0, PIN_CONFIG_MUX_FUNC1),
        _RK3568_UART_TX(GPIO_BANK0, GPIO_PIN_D1, PIN_CONFIG_MUX_FUNC1),
#elif defined(BSP_USING_UART2_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART2, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK1, GPIO_PIN_D6, PIN_CONFIG_MUX_FUNC2),
        _RK3568_UART_TX(GPIO_BANK1, GPIO_PIN_D5, PIN_CONFIG_MUX_FUNC2),
#else
#error "Please define UART2 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART3)
    {
        _RK3568_UART("uart3", UART3, CLK_UART3, BSP_UART3_BAUDRATE, 151),
#if defined(BSP_USING_UART3_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART3, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK1, GPIO_PIN_A0, PIN_CONFIG_MUX_FUNC2),
        _RK3568_UART_TX(GPIO_BANK1, GPIO_PIN_A1, PIN_CONFIG_MUX_FUNC2),
#elif defined(BSP_USING_UART3_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART3, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_C0, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK3, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART3 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART4)
    {
        _RK3568_UART("uart4", UART4, CLK_UART4, BSP_UART4_BAUDRATE, 152),
#if defined(BSP_USING_UART4_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART4, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK1, GPIO_PIN_A4, PIN_CONFIG_MUX_FUNC2),
        _RK3568_UART_TX(GPIO_BANK1, GPIO_PIN_A6, PIN_CONFIG_MUX_FUNC2),
#elif defined(BSP_USING_UART4_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART4, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_B1, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK3, GPIO_PIN_B2, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART4 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART5)
    {
        _RK3568_UART("uart5", UART5, CLK_UART5, BSP_UART5_BAUDRATE, 153),
#if defined(BSP_USING_UART5_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART5, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_A1, PIN_CONFIG_MUX_FUNC3),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_A2, PIN_CONFIG_MUX_FUNC3),
#elif defined(BSP_USING_UART5_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART5, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_C3, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK3, GPIO_PIN_C2, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART5 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART6)
    {
        _RK3568_UART("uart6", UART6, CLK_UART6, BSP_UART6_BAUDRATE, 154),
#if defined(BSP_USING_UART6_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART6, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_A3, PIN_CONFIG_MUX_FUNC3),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_A4, PIN_CONFIG_MUX_FUNC3),
#elif defined(BSP_USING_UART6_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART6, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK1, GPIO_PIN_D6, PIN_CONFIG_MUX_FUNC3),
        _RK3568_UART_TX(GPIO_BANK1, GPIO_PIN_D5, PIN_CONFIG_MUX_FUNC3),
#else
#error "Please define UART6 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART7)
    {
        _RK3568_UART("uart7", UART7, CLK_UART7, BSP_UART7_BAUDRATE, 155),
#if defined(BSP_USING_UART7_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART7, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_A5, PIN_CONFIG_MUX_FUNC3),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_A6, PIN_CONFIG_MUX_FUNC3),
#elif defined(BSP_USING_UART7_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART7, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK3, GPIO_PIN_C4, PIN_CONFIG_MUX_FUNC4),
#elif defined(BSP_USING_UART7_M2)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART7, IOFUNC_SEL_M2),
        _RK3568_UART_RX(GPIO_BANK4, GPIO_PIN_A3, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK4, GPIO_PIN_A2, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART7 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART8)
    {
        _RK3568_UART("uart8", UART8, CLK_UART8, BSP_UART8_BAUDRATE, 156),
#if defined(BSP_USING_UART8_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART8, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_C6, PIN_CONFIG_MUX_FUNC2),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC3),
#elif defined(BSP_USING_UART8_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART8, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK3, GPIO_PIN_A0, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_D7, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART8 Module."
#endif
    },
#endif

#if defined(BSP_USING_UART9)
    {
        _RK3568_UART("uart9", UART9, CLK_UART9, BSP_UART9_BAUDRATE, 157),
#if defined(BSP_USING_UART9_M0)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART9, IOFUNC_SEL_M0),
        _RK3568_UART_RX(GPIO_BANK2, GPIO_PIN_A7, PIN_CONFIG_MUX_FUNC3),
        _RK3568_UART_TX(GPIO_BANK2, GPIO_PIN_B0, PIN_CONFIG_MUX_FUNC3),
#elif defined(BSP_USING_UART9_M1)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART9, IOFUNC_SEL_M1),
        _RK3568_UART_RX(GPIO_BANK4, GPIO_PIN_C6, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK4, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC4),
#elif defined(BSP_USING_UART9_M2)
        _RK3568_UART_FUNC(HAL_PINCTRL_IOFuncSelForUART9, IOFUNC_SEL_M2),
        _RK3568_UART_RX(GPIO_BANK4, GPIO_PIN_A5, PIN_CONFIG_MUX_FUNC4),
        _RK3568_UART_TX(GPIO_BANK4, GPIO_PIN_A4, PIN_CONFIG_MUX_FUNC4),
#else
#error "Please define UART9 Module."
#endif
    },
#endif
};

static void rk3568_uart_isr(uintptr_t arg)
{
    struct rk3568_uart *uart = (struct rk3568_uart *)arg;
    struct rt_serial_device *serial = &uart->serial;
    uint8_t ch;
    while (HAL_UART_SerialIn(uart->reg, &ch, 1) == 1)
    {
        struct rt_serial_rx_fifo *rx_fifo;
        rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
        RT_ASSERT(rx_fifo != RT_NULL);

        rt_ringbuffer_putchar(&(rx_fifo->rb), ch);

        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }
}

static rt_err_t rk3568_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct rk3568_uart *uart;
    struct HAL_UART_DEV uart_dev;
    struct HAL_UART_CONFIG uart_config;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = rt_container_of(serial, struct rk3568_uart, serial);

    uart_dev.pReg = uart->reg;
    uart_dev.sclkID = uart->clk;

    uart_config.baudRate = cfg->baud_rate;

    switch (cfg->data_bits)
    {
    case DATA_BITS_5:
    case DATA_BITS_6:
    case DATA_BITS_7:
    case DATA_BITS_8:
        uart_config.dataBit = cfg->data_bits;
        break;
    default:
        uart_config.dataBit = DATA_BITS_8;
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart_config.stopBit = UART_ONE_STOPBIT;
        break;
    case STOP_BITS_2:
        uart_config.stopBit = UART_ONE_AND_HALF_OR_TWO_STOPBIT;
        break;
    default:
        uart_config.stopBit = UART_ONE_STOPBIT;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_ODD:
        uart_config.parity = UART_ODD_PARITY;
        break;
    case PARITY_EVEN:
        uart_config.parity = UART_EVEN_PARITY;
        break;
    default:
        uart_config.parity = UART_PARITY_DISABLE;
        break;
    }

    HAL_UART_Init(&uart_dev, &uart_config);

    return RT_EOK;
}

static rt_err_t rk3568_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct rk3568_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct rk3568_uart, serial);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        HAL_UART_DisableIrq(uart->reg, UART_IER_RDI);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        HAL_UART_EnableIrq(uart->reg, UART_IER_RDI);
        break;

    case RT_DEVICE_CHECK_OPTMODE:
        return RT_SERIAL_TX_BLOCKING_BUFFER;

    case RT_DEVICE_CTRL_CLOSE:
        HAL_UART_Reset(uart->reg);
        break;
    }

    return RT_EOK;
}

static int rk3568_uart_putc(struct rt_serial_device *serial, char ch)
{
    struct rk3568_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct rk3568_uart, serial);

    HAL_UART_SerialOutChar(uart->reg, ch);

    return 1;
}

static int rk3568_uart_getc(struct rt_serial_device *serial)
{
    struct rk3568_uart *uart;
    uint8_t ch;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct rk3568_uart, serial);

    if (HAL_UART_SerialIn(uart->reg, &ch, 1) == 1)
    {
        return ch;
    }

    return -1;
}

static const struct rt_uart_ops rk3568_uart_ops =
    {
        .configure = rk3568_uart_configure,
        .control = rk3568_uart_control,
        .putc = rk3568_uart_putc,
        .getc = rk3568_uart_getc,
        .transmit = RT_NULL,
};

rt_err_t rk3568_uart_gpio_init(struct rk3568_uart *uart)
{
    if (HAL_PINCTRL_SetIOMUX(uart->rx_bank, uart->rx_pin, uart->rx_param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    if (HAL_PINCTRL_SetIOMUX(uart->tx_bank, uart->tx_pin, uart->tx_param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3568_uart_config_init(struct rk3568_uart *uart)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    uart->serial.config = config;
    uart->serial.config.baud_rate = uart->baud_rate;

    return RT_EOK;
}

rt_err_t rk3568_uart_intr_init(struct rk3568_uart *uart)
{
    U32 ret;

    ret = PRT_HwiSetAttr(uart->intr_num, uart->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(uart->intr_num, rk3568_uart_isr, (uintptr_t)uart);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(uart->intr_num);
    ret = PRT_HwiEnable(uart->intr_num);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    HAL_UART_EnableIrq(uart->reg, UART_IER_RDI);

    return RT_EOK;
}

int drv_uart_init()
{
    for (int i = 0; i < sizeof(uart_obj) / sizeof(uart_obj[0]); i++)
    {
        uart_obj[i].serial.ops = &rk3568_uart_ops;

        int result = rt_hw_serial_register(&uart_obj[i].serial,
                                           uart_obj[i].name,
                                           RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                           (void *)&uart_obj[i]);
        if (result != RT_EOK)
        {
            return -RT_ERROR;
        }

        HAL_UART_Reset(uart_obj[i].reg);

        uart_obj[i].io_func(uart_obj[i].io_mode);

        if (rk3568_uart_gpio_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3568_uart_config_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3568_uart_intr_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
