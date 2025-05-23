/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-25     WuPeiFeng   the first version
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

rt_err_t ds1307_write_byte(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};

    if (rt_i2c_master_send(i2c_bus, dev_addr, 0, buf, 2) != 1)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t ds1307_read_byte(uint8_t reg, uint8_t *data)
{
    uint8_t buf[2] = {reg};

    if (rt_i2c_master_recv(i2c_bus, dev_addr, 0, buf, 1) != 1)
    {
        return -RT_ERROR;
    }

    *data = buf[0];

    return RT_EOK;
}

/* 读取RTC时间 */
int ds1307_get_time(struct tm *time)
{
    uint8_t val[7];

    memset(time, 0, sizeof(struct tm));

    for(int i = 0; i < 7; i++)
    {
        if (ds1307_read_byte(i, val+i) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    time->tm_sec = BCD_TO_DEC(val[0]);
    time->tm_min = BCD_TO_DEC(val[1]);
    time->tm_hour = BCD_TO_DEC(val[2]);
    time->tm_wday = BCD_TO_DEC(val[3]) - 1;
    time->tm_mday = BCD_TO_DEC(val[4]);
    time->tm_mon = BCD_TO_DEC(val[5] & 0x1f) - 1;
    time->tm_year = BCD_TO_DEC(val[6]) + 100;

    return RT_EOK;
}

/* 设置RTC时间 */
int ds1307_set_time(struct tm *time)
{
    if (time->tm_sec < 0 || time->tm_sec > 59 ||
        time->tm_min < 0 || time->tm_min > 59 ||
        time->tm_hour < 0 || time->tm_hour > 23 ||
        time->tm_mday < 1 || time->tm_mday > 31 ||
        time->tm_mday < 0 || time->tm_mday > 6 ||
        time->tm_mon < 0 || time->tm_mon > 11 ||
        time->tm_year < 0 || time->tm_year > 199)
    {
        return -RT_ERROR;
    }

    ds1307_write_byte(0, DEC_TO_BCD(time->tm_sec));
    ds1307_write_byte(1, DEC_TO_BCD(time->tm_min));
    ds1307_write_byte(2, DEC_TO_BCD(time->tm_hour));
    ds1307_write_byte(3, DEC_TO_BCD(time->tm_wday + 1));
    ds1307_write_byte(4, DEC_TO_BCD(time->tm_mday));
    ds1307_write_byte(5, DEC_TO_BCD(time->tm_mon + 1));
    ds1307_write_byte(6, DEC_TO_BCD(time->tm_year - 100));

    return RT_EOK;
}

int ds1307_dev_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(DEMO_I2C_DS1307_DEV);
    if (i2c_bus == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_I2C_DS1307_DEV);
        return -RT_ERROR;
    }

    PRT_Printf("find device %s success\n", DEMO_I2C_DS1307_DEV);

    return RT_EOK;
}

void i2c_ds1307_demo()
{
    if (ds1307_dev_init() != 0)
    {
        PRT_Printf("ds1307_dev_init error\n");
        return;
    }

    struct tm time;
    struct timespec ts;

    /* 设置RTC时间: 2024-12-01 12:00:00 */
    {
        time.tm_year = 2024 - 1900;
        time.tm_mon = 12 -1;
        time.tm_mday = 1;
        time.tm_wday = 0;
        time.tm_hour = 12;
        time.tm_min = 0;
        time.tm_sec = 0;

        if (ds1307_set_time(&time) != RT_EOK)
        {
            PRT_Printf("ds1307_set_time error\n");
            return;
        }
    }

    /* ds1307芯片写操作之后，需要延迟一段时间，否则可能读取失败 */
    PRT_ClkDelayMs(10);

    /* 读取RTC时间，并且更新至系统时间，以便使用posix时间接口 */
    {
        if (ds1307_get_time(&time) != RT_EOK)
        {
            PRT_Printf("ds1307_get_time error\n");
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
