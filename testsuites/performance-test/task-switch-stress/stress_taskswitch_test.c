#include <rtdevice.h>
#include <prt_config.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_mem.h>
#include <print.h>
#include <performance_public.h>

int task_switch_stress()
{
    PRT_Printf("TASK SWITCH STRESS TEST Demo\n");
    int duration_s = 60 ; // 测试持续时间

    /*压力线程*/
    for(int i =0;i<5;i++){
        create_task(cpu_stress, 12, "cpu_stress");
        create_task(mem_stress, 12, "cache_stress");
    // CreateTask(io_stress, 32, "io_stress");
    }
    
    /*被测试任务最后创建*/
    create_task(task_switch, 10, "task_switch");

    sleep(duration_s - 2); // 压力测试进行中
    for (int i = 0; i < stress_cnt - 1; ++i)
    {
        PRT_TaskSuspend(stress_handle[i]); // 挂起压力线程
    }
    sleep(2);
    
    return 0;
}