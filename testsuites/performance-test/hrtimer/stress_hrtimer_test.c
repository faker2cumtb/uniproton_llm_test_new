#include <rtdevice.h>
#include <prt_config.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_mem.h>
#include <print.h>
#include <performance_public.h>

int hrtimer_stress_test()
{
    PRT_Printf("HRTIMER STRESS TEST Demo\n");
    int duration_s = HRTIMER_TEST_TIME + 10 ; // 测试持续时间

    /*压力线程*/
    for(int i =0;i<5;i++){
        create_task(cpu_stress, 32, "cpu_stress");
        create_task(mem_stress, 32, "cache_stress");
        // CreateTask(io_stress, 32, "io_stress");
    }
    /*被测试任务最后创建*/
    create_task(hrtimer_nostress_test, 10, "hrtimer_test_demo");

    sleep(duration_s - 2); // 压力测试进行中
    for (int i = 0; i < stress_cnt - 1; ++i)
    {
        PRT_TaskSuspend(stress_handle[i]); // 挂起压力线程
    }
    sleep(2);
    
    return 0;
}