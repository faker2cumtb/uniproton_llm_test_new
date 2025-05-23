/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2015-08-31     heyuanjie87    first version
 * 2024-07-22     LuoYuncong     Port to UniProton
 * 2024-11-19     LuoYuncong     Improves reading accuracy
 */

#include <rtdevice.h>
#include <string.h>
#include <prt_sys.h>

rt_inline rt_uint32_t timeout_calc(rt_hwtimer_t *timer, rt_hwtimerval_t *tv)
{
    float overflow;
    float timeout;
    rt_uint32_t counter;
    int i, index = 0;
    float tv_sec;
    float devi_min = 1;
    float devi;

    /* changed to second */
    overflow = timer->info->maxcnt / (float)timer->freq;
    tv_sec = tv->sec + tv->usec / (float)1000000;

    if (tv_sec < (1 / (float)timer->freq))
    {
        /* little timeout */
        i = 0;
        timeout = 1 / (float)timer->freq;
    }
    else
    {
        for (i = 1; i > 0; i++)
        {
            timeout = tv_sec / i;

            if (timeout <= overflow)
            {
                counter = (rt_uint32_t)(timeout * timer->freq);
                devi = tv_sec - (counter / (float)timer->freq) * i;
                /* Minimum calculation error */
                if (devi > devi_min)
                {
                    i = index;
                    timeout = tv_sec / i;
                    break;
                }
                else if (devi == 0)
                {
                    break;
                }
                else if (devi < devi_min)
                {
                    devi_min = devi;
                    index = i;
                }
            }
        }
    }

    timer->cycles = i;
    timer->reload = i;
    timer->period_sec = timeout;
    counter = (rt_uint32_t)(timeout * timer->freq);

    return counter;
}

static rt_err_t rt_hwtimer_init(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    rt_hwtimer_t *timer;

    timer = (rt_hwtimer_t *)dev;
    /* try to change to 1MHz */
    if ((1000000 <= timer->info->maxfreq) && (1000000 >= timer->info->minfreq))
    {
        timer->freq = 1000000;
    }
    else
    {
        timer->freq = timer->info->minfreq;
    }
    timer->mode = HWTIMER_MODE_ONESHOT;
    timer->cycles = 0;
    timer->overflow = 0;

    if (timer->ops->init)
    {
        timer->ops->init(timer, 1);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}

static rt_err_t rt_hwtimer_open(struct rt_device *dev, rt_uint16_t oflag)
{
    rt_err_t result = RT_EOK;
    rt_hwtimer_t *timer;

    timer = (rt_hwtimer_t *)dev;
    if (timer->ops->control != RT_NULL)
    {
        timer->ops->control(timer, HWTIMER_CTRL_FREQ_SET, &timer->freq);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}

static rt_err_t rt_hwtimer_close(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    rt_hwtimer_t *timer;

    timer = (rt_hwtimer_t *)dev;
    if (timer->ops->init != RT_NULL)
    {
        timer->ops->init(timer, 0);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    dev->flag &= ~RT_DEVICE_FLAG_ACTIVATED;
    dev->rx_indicate = RT_NULL;

    return result;
}

static rt_size_t rt_hwtimer_read(struct rt_device *dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_hwtimer_t *timer;
    rt_hwtimerval_t tv;
    rt_uint32_t cnt;
    rt_int32_t overflow;

    timer = (rt_hwtimer_t *)dev;
    if (timer->ops->count_get == RT_NULL)
        return 0;

    cnt = timer->ops->count_get(timer);
    overflow = timer->overflow;

    if (timer->info->cntmode == HWTIMER_CNTMODE_DW)
    {
        cnt = (rt_uint32_t)(timer->freq * timer->period_sec) - cnt;
    }
    if (timer->mode == HWTIMER_MODE_ONESHOT)
    {
        overflow = 0;
    }

#if defined(OS_ARCH_ARMV8) || defined(OS_ARCH_X86_64) || defined(OS_ARCH_RISCV64)
    rt_uint64_t period = timer->period_sec * OS_SYS_US_PER_SECOND;
    rt_uint64_t freq = timer->freq / OS_SYS_US_PER_SECOND;
    rt_uint64_t t = overflow * period + cnt / freq;

    tv.sec = (rt_int32_t)(t / OS_SYS_US_PER_SECOND);
    tv.usec = (rt_int32_t)(t % OS_SYS_US_PER_SECOND);
#else
    float t = overflow * timer->period_sec + cnt / (float)timer->freq;
    tv.sec = (rt_int32_t)t;
    tv.usec = (rt_int32_t)((t - tv.sec) * OS_SYS_US_PER_SECOND);
#endif

    size = size > sizeof(tv) ? sizeof(tv) : size;
    memcpy(buffer, &tv, size);

    return size;
}

static rt_size_t rt_hwtimer_write(struct rt_device *dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_uint32_t t;
    rt_hwtimer_mode_t opm = HWTIMER_MODE_PERIOD;
    rt_hwtimer_t *timer;

    timer = (rt_hwtimer_t *)dev;
    if ((timer->ops->start == RT_NULL) || (timer->ops->stop == RT_NULL))
        return 0;

    if (size != sizeof(rt_hwtimerval_t))
        return 0;

    timer->ops->stop(timer);

    timer->overflow = 0;

    t = timeout_calc(timer, (rt_hwtimerval_t *)buffer);
    if ((timer->cycles <= 1) && (timer->mode == HWTIMER_MODE_ONESHOT))
    {
        opm = HWTIMER_MODE_ONESHOT;
    }

    if (timer->ops->start(timer, t, opm) != RT_EOK)
        size = 0;

    return size;
}

static rt_err_t rt_hwtimer_control(struct rt_device *dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    rt_hwtimer_t *timer;

    timer = (rt_hwtimer_t *)dev;

    switch (cmd)
    {
    case HWTIMER_CTRL_STOP:
    {
        if (timer->ops->stop != RT_NULL)
        {
            timer->ops->stop(timer);
        }
        else
        {
            result = -RT_ENOSYS;
        }
    }
    break;
    case HWTIMER_CTRL_FREQ_SET:
    {
        rt_int32_t *f;

        if (args == RT_NULL)
        {
            result = -RT_EEMPTY;
            break;
        }

        f = (rt_int32_t *)args;
        if ((*f > timer->info->maxfreq) || (*f < timer->info->minfreq))
        {
            result = -RT_EINVAL;
            break;
        }

        if (timer->ops->control != RT_NULL)
        {
            result = timer->ops->control(timer, cmd, args);
            if (result == RT_EOK)
            {
                timer->freq = *f;
            }
        }
        else
        {
            result = -RT_ENOSYS;
        }
    }
    break;
    case HWTIMER_CTRL_INFO_GET:
    {
        if (args == RT_NULL)
        {
            result = -RT_EEMPTY;
            break;
        }

        *((struct rt_hwtimer_info *)args) = *timer->info;
    }
    break;
    case HWTIMER_CTRL_MODE_SET:
    {
        rt_hwtimer_mode_t *m;

        if (args == RT_NULL)
        {
            result = -RT_EEMPTY;
            break;
        }

        m = (rt_hwtimer_mode_t *)args;

        if ((*m != HWTIMER_MODE_ONESHOT) && (*m != HWTIMER_MODE_PERIOD))
        {
            result = -RT_ERROR;
            break;
        }
        timer->mode = *m;
    }
    break;
    default:
    {
        if (timer->ops->control != RT_NULL)
        {
            result = timer->ops->control(timer, cmd, args);
        }
        else
        {
            result = -RT_ENOSYS;
        }
    }
    break;
    }

    return result;
}

void rt_device_hwtimer_isr(rt_hwtimer_t *timer)
{
    RT_ASSERT(timer != RT_NULL);

    timer->overflow++;

    if (timer->cycles != 0)
    {
        timer->cycles--;
    }

    if (timer->cycles == 0)
    {
        timer->cycles = timer->reload;

        if (timer->mode == HWTIMER_MODE_ONESHOT)
        {
            if (timer->ops->stop != RT_NULL)
            {
                timer->ops->stop(timer);
            }
        }

        if (timer->parent.rx_indicate != RT_NULL)
        {
            timer->parent.rx_indicate(&timer->parent, sizeof(struct rt_hwtimerval));
        }
    }
}

rt_err_t rt_device_hwtimer_register(rt_hwtimer_t *timer, const char *name, void *user_data)
{
    struct rt_device *device;

    RT_ASSERT(timer != RT_NULL);
    RT_ASSERT(timer->ops != RT_NULL);
    RT_ASSERT(timer->info != RT_NULL);

    device = &(timer->parent);

    device->type = RT_Device_Class_Timer;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init = rt_hwtimer_init;
    device->open = rt_hwtimer_open;
    device->close = rt_hwtimer_close;
    device->read = rt_hwtimer_read;
    device->write = rt_hwtimer_write;
    device->control = rt_hwtimer_control;

    device->user_data = user_data;

    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
