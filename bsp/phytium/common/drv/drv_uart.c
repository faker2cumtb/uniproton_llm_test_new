/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-22     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <fiopad.h>
#include <fpl011.h>
#include "fmio.h"
#include "fmio_hw.h"
#include "drv_common.h"
#include "drv_uart.h"

struct e2000q_uart
{
    char *name;

    FPl011 ctrl;
    s32 uart_id;
    s32 mio_id;
    u32 baud_rate;

    u32 rx_reg_off;
    u32 tx_reg_off;
    FIOPadFunc func;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    struct rt_serial_device serial;
};

#define _E2000Q_UART(_name, _uart_id, _mio_id, _baud_rate, _rx_reg_off, _tx_reg_off, _func, _intr_num) \
    {                                                                                                  \
        .name = (_name),                                                                               \
        .uart_id = (_uart_id),                                                                         \
        .mio_id = (_mio_id),                                                                           \
        .baud_rate = (_baud_rate),                                                                     \
        .rx_reg_off = (_rx_reg_off),                                                                   \
        .tx_reg_off = (_tx_reg_off),                                                                   \
        .func = (_func),                                                                               \
        .intr_num = (_intr_num),                                                                       \
        .intr_prio = 6,                                                                                \
    }

static struct e2000q_uart uart_obj[] = {

#if defined(BSP_USING_UART0)
    _E2000Q_UART("uart0", FUART0_ID, -1, BSP_UART0_BAUDRATE, FIOPAD_J37_REG0_OFFSET, FIOPAD_J39_REG0_OFFSET, FIOPAD_FUNC4, FUART0_IRQ_NUM),
#endif

#if defined(BSP_USING_UART1)
    _E2000Q_UART("uart1", FUART1_ID, -1, BSP_UART1_BAUDRATE, FIOPAD_AW51_REG0_OFFSET, FIOPAD_AU51_REG0_OFFSET, FIOPAD_FUNC0, FUART1_IRQ_NUM),
#endif

#if defined(BSP_USING_UART2)
    _E2000Q_UART("uart2", FUART2_ID, -1, BSP_UART2_BAUDRATE, FIOPAD_A47_REG0_OFFSET, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC0, FUART2_IRQ_NUM),
#endif

#if defined(BSP_USING_UART3)
    _E2000Q_UART("uart3", FUART3_ID, -1, BSP_UART3_BAUDRATE, FIOPAD_L37_REG0_OFFSET, FIOPAD_N35_REG0_OFFSET, FIOPAD_FUNC2, FUART3_IRQ_NUM),
#endif

#if defined(BSP_USING_MIO_UART0)
    _E2000Q_UART("mio_uart0", -1, FMIO0_ID, BSP_MIO_UART0_BAUDRATE, FIOPAD_A41_REG0_OFFSET, FIOPAD_A43_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO0_ID)),
#endif

#if defined(BSP_USING_MIO_UART1)
    _E2000Q_UART("mio_uart1", -1, FMIO1_ID, BSP_MIO_UART1_BAUDRATE, FIOPAD_A45_REG0_OFFSET, FIOPAD_C45_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO1_ID)),
#endif

#if defined(BSP_USING_MIO_UART2)
    _E2000Q_UART("mio_uart2", -1, FMIO2_ID, BSP_MIO_UART2_BAUDRATE, FIOPAD_A47_REG0_OFFSET, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO2_ID)),
#endif

#if defined(BSP_USING_MIO_UART3)
    _E2000Q_UART("mio_uart3", -1, FMIO3_ID, BSP_MIO_UART3_BAUDRATE, FIOPAD_BA55_REG0_OFFSET, FIOPAD_BA53_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO3_ID)),
#endif

#if defined(BSP_USING_MIO_UART4)
    _E2000Q_UART("mio_uart4", -1, FMIO4_ID, BSP_MIO_UART4_BAUDRATE, FIOPAD_E31_REG0_OFFSET, FIOPAD_G31_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO4_ID)),
#endif

#if defined(BSP_USING_MIO_UART5)
    _E2000Q_UART("mio_uart5", -1, FMIO5_ID, BSP_MIO_UART5_BAUDRATE, FIOPAD_N41_REG0_OFFSET, FIOPAD_N39_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO5_ID)),
#endif

#if defined(BSP_USING_MIO_UART6)
    _E2000Q_UART("mio_uart6", -1, FMIO6_ID, BSP_MIO_UART6_BAUDRATE, FIOPAD_AA57_REG0_OFFSET, FIOPAD_AA59_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO6_ID)),
#endif

#if defined(BSP_USING_MIO_UART7)
    _E2000Q_UART("mio_uart7", -1, FMIO7_ID, BSP_MIO_UART7_BAUDRATE, FIOPAD_L33_REG0_OFFSET, FIOPAD_N45_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO7_ID)),
#endif

#if defined(BSP_USING_MIO_UART8)
    _E2000Q_UART("mio_uart8", -1, FMIO8_ID, BSP_MIO_UART8_BAUDRATE, FIOPAD_AA49_REG0_OFFSET, FIOPAD_W49_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO8_ID)),
#endif

#if defined(BSP_USING_MIO_UART9)
    _E2000Q_UART("mio_uart9", -1, FMIO9_ID, BSP_MIO_UART9_BAUDRATE, FIOPAD_AA51_REG0_OFFSET, FIOPAD_U49_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO9_ID)),
#endif

#if defined(BSP_USING_MIO_UART10)
    _E2000Q_UART("mio_uart10", -1, FMIO10_ID, BSP_MIO_UART10_BAUDRATE, FIOPAD_C49_REG0_OFFSET, FIOPAD_A51_REG0_OFFSET, FIOPAD_FUNC5, FMIO_IRQ_NUM(FMIO10_ID)),
#endif

#if defined(BSP_USING_MIO_UART11)
    _E2000Q_UART("mio_uart11", -1, FMIO11_ID, BSP_MIO_UART11_BAUDRATE, FIOPAD_G59_REG0_OFFSET, FIOPAD_J59_REG0_OFFSET, FIOPAD_FUNC4, FMIO_IRQ_NUM(FMIO11_ID)),
#endif

#if defined(BSP_USING_MIO_UART12)
    _E2000Q_UART("mio_uart12", -1, FMIO12_ID, BSP_MIO_UART12_BAUDRATE, FIOPAD_E41_REG0_OFFSET, FIOPAD_L45_REG0_OFFSET, FIOPAD_FUNC3, FMIO_IRQ_NUM(FMIO12_ID)),
#endif

#if defined(BSP_USING_MIO_UART13)
    _E2000Q_UART("mio_uart13", -1, FMIO13_ID, BSP_MIO_UART13_BAUDRATE, FIOPAD_J49_REG0_OFFSET, FIOPAD_N49_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO13_ID)),
#endif

#if defined(BSP_USING_MIO_UART14)
    _E2000Q_UART("mio_uart14", -1, FMIO14_ID, BSP_MIO_UART14_BAUDRATE, FIOPAD_L51_REG0_OFFSET, FIOPAD_L49_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO14_ID)),
#endif

#if defined(BSP_USING_MIO_UART15)
    _E2000Q_UART("mio_uart15", -1, FMIO15_ID, BSP_MIO_UART15_BAUDRATE, FIOPAD_N53_REG0_OFFSET, FIOPAD_J53_REG0_OFFSET, FIOPAD_FUNC6, FMIO_IRQ_NUM(FMIO15_ID)),
#endif

};

static void e2000q_uart_intr_entry(uintptr_t arg)
{
    struct e2000q_uart *uart = (struct e2000q_uart *)arg;

    FPl011InterruptHandler(0, &uart->ctrl);
}

static void e2000q_uart_intr_handler(void *arg, u32 event, u32 event_data)
{
    struct e2000q_uart *uart = (struct e2000q_uart *)arg;
    struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *)uart->serial.serial_rx;
    uint8_t ch;

    FPl011Receive(&uart->ctrl, &ch, 1);
    rt_ringbuffer_putchar(&(rx_fifo->rb), ch);
    rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_RX_IND);
}

static rt_err_t e2000q_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct e2000q_uart *uart = rt_container_of(serial, struct e2000q_uart, serial);

    FPl011Config config;
    FPl011Format format;

    format.baudrate = cfg->baud_rate;

    switch (cfg->data_bits)
    {
    case DATA_BITS_5:
    case DATA_BITS_6:
    case DATA_BITS_7:
    case DATA_BITS_8:
        format.data_bits = cfg->data_bits - 5;
        break;
    default:
        return -RT_ERROR;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
    case STOP_BITS_2:
        format.stopbits = cfg->stop_bits;
        break;
    default:
        return -RT_ERROR;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        format.parity = FPL011_FORMAT_NO_PARITY;
        break;
    case PARITY_ODD:
        format.parity = FPL011_FORMAT_EN_PARITY | FPL011_FORMAT_ODD_PARITY;
        break;
    case PARITY_EVEN:
        format.parity = FPL011_FORMAT_EN_PARITY | FPL011_FORMAT_EVEN_PARITY;
        break;
    default:
        return -RT_ERROR;
    }

    if (uart->uart_id >= 0)
    {
        config = *FPl011LookupConfig(uart->uart_id);
    }

    if (uart->mio_id >= 0)
    {
        FMioCtrl mio_ctrl;

        mio_ctrl.config = *FMioLookupConfig(uart->mio_id);
        if (FMioFuncInit(&mio_ctrl, FMIO_FUNC_SET_UART) != FT_SUCCESS)
        {
            return -RT_ERROR;
        }

        /* 获取uart1的默认值做参考，并获取MIO的基地址和时钟频率 */
        config = *FPl011LookupConfig(FUART1_ID);
        config.base_address = FMioFuncGetAddress(&mio_ctrl, FMIO_FUNC_SET_UART);
        config.ref_clock_hz = FMIO_CLK_FREQ_HZ;
    }

    if (FPl011CfgInitialize(&uart->ctrl, &config) != FT_SUCCESS)
    {
        return -RT_ERROR;
    }

    if (FPl011SetDataFormat(&uart->ctrl, &format) != FT_SUCCESS)
    {
        return -RT_ERROR;
    }

    FPl011SetOptions(&uart->ctrl, FPL011_OPTION_UARTEN | FPL011_OPTION_FIFOEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN);
    FPl011SetRxFifoThreadhold(&uart->ctrl, FPL011IFLS_RXIFLSEL_3_4);
    FPl011SetTxFifoThreadHold(&uart->ctrl, FPL011IFLS_TXIFLSEL_1_2);
    FPl011SetOperMode(&uart->ctrl, FPL011_OPER_MODE_NORMAL);

    FPl011SetHandler(&uart->ctrl, e2000q_uart_intr_handler, uart);
    FPl011SetInterruptMask(&uart->ctrl, FPL011IMSC_RXIM | FPL011IMSC_RTIM);
    FPl011InterruptClearAll(&uart->ctrl);

    return RT_EOK;
}

static rt_err_t e2000q_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct e2000q_uart *uart = rt_container_of(serial, struct e2000q_uart, serial);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        FPl011SetInterruptMask(&uart->ctrl, 0);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        FPl011SetInterruptMask(&uart->ctrl, FPL011IMSC_RXIM | FPL011IMSC_RTIM);
        FPl011InterruptClearAll(&uart->ctrl);
        break;

    case RT_DEVICE_CHECK_OPTMODE:
        return RT_SERIAL_TX_BLOCKING_BUFFER;

    case RT_DEVICE_CTRL_CLOSE:
        FPl011SetOptions(&uart->ctrl, 0);
        break;
    }

    return RT_EOK;
}

static int e2000q_uart_putc(struct rt_serial_device *serial, char ch)
{
    struct e2000q_uart *uart = rt_container_of(serial, struct e2000q_uart, serial);

    FPl011Send(&uart->ctrl, (u8 *)&ch, 1);

    return 1;
}

static int e2000q_uart_getc(struct rt_serial_device *serial)
{
    struct e2000q_uart *uart = rt_container_of(serial, struct e2000q_uart, serial);
    uint8_t ch;

    if (FPl011Receive(&uart->ctrl, &ch, 1) == 1)
    {
        return ch;
    }

    return -1;
}

static const struct rt_uart_ops e2000q_uart_ops = {
    .configure = e2000q_uart_configure,
    .control = e2000q_uart_control,
    .putc = e2000q_uart_putc,
    .getc = e2000q_uart_getc,
    .transmit = RT_NULL,
};

rt_err_t e2000q_uart_gpio_init(struct e2000q_uart *uart)
{
    FIOPadSetFunc(&iopad_ctrl, uart->rx_reg_off, uart->func);
    FIOPadSetFunc(&iopad_ctrl, uart->tx_reg_off, uart->func);

    return RT_EOK;
}

rt_err_t e2000q_uart_config_init(struct e2000q_uart *uart)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    uart->serial.config = config;
    uart->serial.config.baud_rate = uart->baud_rate;

    return RT_EOK;
}

rt_err_t e2000q_uart_intr_init(struct e2000q_uart *uart)
{
    U32 ret;

    ret = PRT_HwiSetAttr(uart->intr_num, uart->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(uart->intr_num, e2000q_uart_intr_entry, (uintptr_t)uart);
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

    return RT_EOK;
}

int drv_uart_init()
{
    for (int i = 0; i < sizeof(uart_obj) / sizeof(uart_obj[0]); i++)
    {
        uart_obj[i].serial.ops = &e2000q_uart_ops;

        int result = rt_hw_serial_register(&uart_obj[i].serial,
                                           uart_obj[i].name,
                                           RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                           (void *)&uart_obj[i]);
        if (result != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_uart_gpio_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_uart_config_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_uart_intr_init(&uart_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
