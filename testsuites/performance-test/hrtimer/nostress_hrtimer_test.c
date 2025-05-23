#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <prt_sys_external.h>
#include <prt_sem.h>
#include <performance_public.h>
#include <rtdevice.h>

static rt_device_t timer_dev = RT_NULL;
static SemHandle timer_sem;
int printcount = 1;

S64 t1, t1_last, t1_jitter;
S64 t2, t2_last, t2_jitter;
S64 t1_max = 0, t1_min = 0;
S64 t2_max = 0, t2_min = 0;
U64 t1_jitter_sum = 0, t2_jitter_sum = 0;
U64 t1_jitter_region[REGION_LENGTH_HALF * 2];
U64 t2_jitter_region[REGION_LENGTH_HALF * 2];

U32 OsSysClock; // 使用前初始化
static inline S64 cycles2ns(S64 cycles)
{
    return cycles * (S64)NS_PER_SEC / OsSysClock;
}

static inline void code_delay(int t)
{
    while (t--)
    {
        for (int i = 0; i < 1000; i++)
            ;
    }
}

static rt_err_t my_timeout_callback(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(timer_sem);
    return 0;
}

static int TIMER_Init(U64 task_us)
{
    int ret;
    rt_hwtimer_mode_t mode;

    ret = PRT_SemCreate(0, &timer_sem);
    if (ret != OS_OK)
    {
        PRT_Printf("creeate timer_sem failed\n");
        return -1;
    }

    timer_dev = rt_device_find("timer0");
    if (timer_dev == RT_NULL)
    {
        PRT_Printf("timer device not found!\n");
        return -1;
    }

    rt_device_open(timer_dev, RT_DEVICE_OFLAG_RDWR);

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(timer_dev, my_timeout_callback);

    mode = HWTIMER_MODE_PERIOD;
    rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, &mode);

    rt_hwtimerval_t timeout_s;
    timeout_s.sec = 0;
    timeout_s.usec = task_us;
    rt_device_write(timer_dev, 0, &timeout_s, sizeof(timeout_s));
}

int hrtimer_nostress_test()
{
    PRT_Printf("HRTIMER NOSTRESS TEST Demo\n");

    int times = HRTIMER_TEST_TIME * US_PER_SEC / TASK_DURATION_US; // 测试次数
    bool first = true;
    OsSysClock = OsSysGetClock();                                      // 系统时钟频率

    U64 task_cycles = TASK_DURATION_US * (U64)OsSysClock / US_PER_SEC; // 定时器周期数

    PRT_Printf("OsSysGetClock():%lu\n", OsSysClock);
    PRT_Printf("task_cycles:%d\n", task_cycles);
    PRT_Printf("test duration: %ds\n", HRTIMER_TEST_TIME);
    PRT_Printf("=======begin========\n\n");
    sleep(1);

    TIMER_Init(TASK_DURATION_US); // 开启周期定时器

    // 周期任务
    for (int i = 0; i < times + 1; ++i)
    {
        PRT_SemPend(timer_sem, OS_WAIT_FOREVER);
        t1 = PRT_ClkGetCycleCount64();
        code_delay(5); // 注意不同板子执行时间不同
        t2 = PRT_ClkGetCycleCount64();
        if (!first)
        {
            t1_jitter = t1 - t1_last - task_cycles;
            t2_jitter = t2 - t2_last - task_cycles;
            t1_max = (t1_jitter > t1_max) ? (t1_jitter) : (t1_max);
            t1_min = (t1_jitter < t1_min) ? (t1_jitter) : (t1_min);
            t2_max = (t2_jitter > t2_max) ? (t2_jitter) : (t2_max);
            t2_min = (t2_jitter < t2_min) ? (t2_jitter) : (t2_min);
            if (t1_jitter > -REGION_LENGTH_HALF && t1_jitter < REGION_LENGTH_HALF)
            {
                t1_jitter_region[t1_jitter + REGION_LENGTH_HALF]++;
            }
            if (t2_jitter > -REGION_LENGTH_HALF && t2_jitter < REGION_LENGTH_HALF)
            {
                t2_jitter_region[t2_jitter + REGION_LENGTH_HALF]++;
            }
            t1_jitter_sum += abs(t1_jitter);
            t2_jitter_sum += abs(t2_jitter);
        }
        t1_last = t1;
        t2_last = t2;
        first = false;
    }

    rt_device_close(timer_dev);

    // 统计结果打印
    while (printcount--)
    {
        {
            PRT_Printf("\nt1_jitter_region>\n");
            int cnt = 0;
            for (int i = 0; i < REGION_LENGTH_HALF * 2; ++i)
            {
                if (t1_jitter_region[i] != 0)
                {
                    PRT_Printf("%d:%d\n", i - REGION_LENGTH_HALF, t1_jitter_region[i]);
                    cnt++;
                }
            }

            PRT_Printf("\n");
            PRT_Printf("t2_jitter_region>\n");
            for (int i = 0; i < REGION_LENGTH_HALF * 2; ++i)
            {
                if (t2_jitter_region[i] != 0)
                {
                    PRT_Printf("%d:%d\n", i - REGION_LENGTH_HALF, t2_jitter_region[i]);
                }
            }
            PRT_Printf("\n");
        }
        PRT_Printf("total duration: %ds\n", HRTIMER_TEST_TIME);
        PRT_Printf("task duration: %dus\n", TASK_DURATION_US);
        PRT_Printf("task cycles: %d\n", task_cycles);
        PRT_Printf("times: %d\n", times);
        PRT_Printf("t1_min:%dns\nt2_min:%dns\n", cycles2ns(t1_min), cycles2ns(t2_min));
        PRT_Printf("t1_max:%dns\nt2_max:%dns\n", cycles2ns(t1_max), cycles2ns(t2_max));
        PRT_Printf("t1_avg:%dns\nt2_avg:%dns\n", cycles2ns(t1_jitter_sum) / times, cycles2ns(t2_jitter_sum) / times);
        PRT_Printf("\n\n");
        sleep(30);
    }
}