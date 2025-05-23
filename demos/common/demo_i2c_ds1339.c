/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-27     LuoYuncong   the first version
 * 2024-10-29     LuoYuncong   add rtc demo
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <demo.h>

#define BCD_TO_DEC(bcd) (((bcd) & 0x0F) + (((bcd) >> 4) * 10))
#define DEC_TO_BCD(dec) ((((dec) / 10) << 4) | ((dec) % 10))

static struct rt_i2c_bus_device *i2c_bus;
static uint8_t dev_addr = 0x68;

static rt_err_t ds1339_write_regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t buf[20] = {reg};

    if (len > 16)
    {
        return -RT_ERROR;
    }

    memcpy(buf + 1, data, len);

    len++;
    if (rt_i2c_master_send(i2c_bus, dev_addr, 0, buf, len) != len)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t ds1339_read_regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    if (len > 16)
    {
        return -RT_ERROR;
    }

    /* 首字节是i2c设备寄存器 */
    *data = reg;

    if (rt_i2c_master_recv(i2c_bus, dev_addr, 0, data, len) != len)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/* 读取RTC时间 */
int ds1339_get_time(struct tm *time)
{
    uint8_t val[7];

    memset(time, 0, sizeof(struct tm));

    if (ds1339_read_regs(0, val, sizeof(val)) != RT_EOK)
    {
        return -RT_ERROR;
    }

    time->tm_sec = BCD_TO_DEC(val[0]);
    time->tm_min = BCD_TO_DEC(val[1]);
    time->tm_hour = BCD_TO_DEC(val[2]);
    time->tm_wday = BCD_TO_DEC(val[3]) - 1;
    time->tm_mday = BCD_TO_DEC(val[4]);
    time->tm_mon = BCD_TO_DEC(val[5] & 0x7f) - 1;
    time->tm_year = BCD_TO_DEC(val[6]) + ((val[5] >> 7) & 1) * 100;

    return RT_EOK;
}

/* 设置RTC时间 */
int ds1339_set_time(struct tm *time)
{
    uint8_t val[7];

    if (time->tm_sec < 0 || time->tm_sec > 59 ||
        time->tm_min < 0 || time->tm_min > 59 ||
        time->tm_hour < 0 || time->tm_hour > 23 ||
        time->tm_mday < 1 || time->tm_mday > 31 ||
        time->tm_mon < 0 || time->tm_mon > 11 ||
        time->tm_year < 0 || time->tm_year > 199)
    {
        return -RT_ERROR;
    }

    val[0] = DEC_TO_BCD(time->tm_sec);
    val[1] = DEC_TO_BCD(time->tm_min);
    val[2] = DEC_TO_BCD(time->tm_hour);
    val[3] = DEC_TO_BCD(time->tm_wday + 1);
    val[4] = DEC_TO_BCD(time->tm_mday);
    val[5] = DEC_TO_BCD(time->tm_mon + 1);
    val[6] = DEC_TO_BCD(time->tm_year - 100);

    if (time->tm_year > 99)
    {
        val[5] |= 0x80;
    }

    ds1339_write_regs(0, val, sizeof(val));

    return RT_EOK;
}

int ds1339_dev_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(DEMO_I2C_DS1339_DEV);
    if (i2c_bus == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_I2C_DS1339_DEV);
        return -RT_ERROR;
    }

    PRT_Printf("find device %s success\n", DEMO_I2C_DS1339_DEV);

    return RT_EOK;
}

void i2c_ds1339_demo()
{
    if (ds1339_dev_init() != 0)
    {
        PRT_Printf("ds1339_dev_init error\n");
        return;
    }

    struct tm time;
    struct timespec ts;

    /* 设置RTC时间: 2024-11-01 12:00:00 */
    {
        time.tm_year = 2024 - 1900;
        time.tm_mon = 11 - 1;
        time.tm_mday = 1;
        time.tm_wday = 0;
        time.tm_hour = 12;
        time.tm_min = 0;
        time.tm_sec = 0;

        if (ds1339_set_time(&time) != RT_EOK)
        {
            PRT_Printf("ds1339_set_time error\n");
            return;
        }
    }

    /* DS1339芯片写操作之后，需要延迟一段时间，否则可能读取失败 */
    PRT_ClkDelayMs(10);

    /* 读取RTC时间，并且更新至系统时间，以便使用posix时间接口 */
    {
        if (ds1339_get_time(&time) != RT_EOK)
        {
            PRT_Printf("ds1339_get_time error\n");
        }

        ts.tv_sec = mktime(&time);
        ts.tv_nsec = 0;
        clock_settime(CLOCK_REALTIME, &ts);
    }

    /* 读取系统时间，并显示出来 */
    for (int i = 0; i < 10; i++)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        struct tm *p_time = gmtime(&ts.tv_sec);

        PRT_Printf("RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
                   p_time->tm_year + 1900,
                   p_time->tm_mon + 1,
                   p_time->tm_mday,
                   p_time->tm_hour,
                   p_time->tm_min,
                   p_time->tm_sec);

        sleep(1);
    }
}
