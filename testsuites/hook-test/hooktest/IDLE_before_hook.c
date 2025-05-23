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
#include <time.h>
#include <unistd.h>
#include "prt_idle.h"
#include "runHookTest.h"

static void idle_callback(void)
{
    // 获取当前时间的时间戳
    print_time();
    hook_count++;
    printf("enter hook_callback count: %d\n", hook_count);
}

int idle_before_hook_test()
{
    printf("start idle_before_hook_test\n");
    hook_count = 0;
    PRT_IdleAddPrefixHook(idle_callback);

    sleep(10);

    if (hook_count == 1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
        printf("hook_count val is %d, not right\n", hook_count);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

}