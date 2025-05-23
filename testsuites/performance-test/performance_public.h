/*
 * Description: performance测试定义
 */
#include "fcntl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "prt_buildef.h"
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include <unistd.h>

#define US_PER_SEC 1000000ULL
#define NS_PER_SEC 1000000000ULL
#define HRTIMER_TEST_TIME 60 * 4 * 1  // 测试持续时间，单位s
#define TASK_DURATION_US 200 // 定时周期，单位us
#define REGION_LENGTH_HALF 10000

extern TskHandle stress_handle[100]; 
extern int stress_cnt;          

extern int hrtimer_nostress_test();
extern int hrtimer_stress_test();
extern U32 create_task(void *pfunc, TskPrior prio, char *name);
extern void cpu_stress();
extern void mem_stress();
extern void io_stress();
extern int release_mutex_test();
extern void interrupt_main();
extern int interrupt_gpio_test();
extern int task_switch_stress();
extern void task_switch();
