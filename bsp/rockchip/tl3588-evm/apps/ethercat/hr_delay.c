#include <prt_typedef.h>
#include <prt_hwi.h>
#include <prt_sem.h>
#include <prt_clk.h>
#include <prt_sys_external.h>
#include <rtdevice.h>
#include <print.h>

#define TEST_TIMER TIMER1
#define TEST_TIMER_INT 322

static SemHandle timer_sem;
rt_device_t timer_dev = RT_NULL;

static rt_err_t timeout_callback(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(timer_sem);

    return 0;
}

int TIMER_Init()
{
    int ret;
    rt_hwtimer_mode_t mode;

    ret = PRT_SemCreate(0, &timer_sem);
    if (ret != OS_OK)
    {
        PRT_Printf("creeate timer_sem failed\n");
        return -1;
    }

    timer_dev = rt_device_find("timer6");

    if (timer_dev == RT_NULL)
    {
        PRT_Printf("timer device not found!\n");
        return -1;
    }

    rt_device_open(timer_dev, RT_DEVICE_OFLAG_RDWR);

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(timer_dev, timeout_callback);

    mode = HWTIMER_MODE_ONESHOT;
    rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, &mode);

    return 0;
}

#define US_PER_SEC 1000000ULL
int hrdelay(U32 cycles)
{
    if (timer_dev == RT_NULL)
    {
        PRT_Printf("timer_dev not init\n");
        return -1;
    }
    rt_hwtimerval_t timeout_s;
    timeout_s.sec = 0;
    timeout_s.usec = (U64)cycles * US_PER_SEC / OsSysGetClock();
    rt_device_write(timer_dev, 0, &timeout_s, sizeof(timeout_s));

    PRT_SemPend(timer_sem, OS_WAIT_FOREVER);

    return 0;
}