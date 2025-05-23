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

static void idle_hook(void)
{
	if(hook_count % 1000000 == 0 ){
		print_time();
		printf("enter hook_callback count: %d\n", hook_count);
	}
	
	hook_count++;
}

int idle_hook_test()
{
    hook_count = 0;
    PRT_IdleHookAdd(idle_hook);

    sleep(1);
	PRT_IdleHookDel(idle_hook);
    if (hook_count > 1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}