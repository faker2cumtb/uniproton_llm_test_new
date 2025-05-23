#include "fcntl.h"
#include <stdio.h>
#include <test.h>
#include <fstest.h>

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

#define _XOPEN_SOURCE 600
#include <unistd.h>

int test_restart_flag;
T_TestResults c_Results;

int tstdef_CreateFile(char* ch)
{
	FILE * fd =NULL;
	int ret;
	fd = fopen(ch,"w+");
	if(fd != NULL)
	{
		ret = fclose(fd);
		if(ret ==0)
		{
			return AFILE_OK;
		}
		TEST_FAILRINT();
		return AFILE_FAILED;
	}
	else{
		TEST_FAILRINT();
		return AFILE_FAILED;
	}
}
int tstdef_CreateAndWriteFile(char* ch,char* str)
{
	FILE * fd =NULL;
	int ret;
	fd = fopen(ch,"w+");
	if(fd != NULL)
	{
		ret = fwrite(str,sizeof(char),strlen(str),fd);
		if(ret != strlen(str))
		{
			TEST_FAILRINT();
			fclose(fd);
			remove(ch);
			return AFILE_FAILED;
		}
		ret = fclose(fd);
		if(ret ==0)
		{
			return AFILE_OK;
		}
		TEST_FAILRINT();
		return AFILE_FAILED;
	}
	else{
		TEST_FAILRINT();
		return AFILE_FAILED;
	}
}

void test_begin(char *casename)
{
printf("****test_begin****%s\n",casename);
}
void test_end(char *casename)
{
printf("****test_end****%s\n",casename);
}

long sysconf(int name)
{
    switch(name) {
        case _SC_CPUTIME:
        case _SC_THREAD_CPUTIME:
        case _SC_MONOTONIC_CLOCK:
            return 1;
        case _SC_SEM_NSEMS_MAX:
            return OS_SEM_COUNT_MAX;
        default:
            return 0;
    }
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int i;
    int ret = 0;
    test_run_main *run;
	mica_service_init();

    PRT_Printf("Start testing fs....\n");

    for (i = 0; i < sizeof(run_test_arry_1)/sizeof(test_run_main *); i++) {
        run = run_test_arry_1[i];
        PRT_Printf("Runing %s test...\n", run_test_name_1[i]);
        ret = run();
        if (ret != 0) {
            failCount++;
            PRT_Printf("Run %s test fail\n", run_test_name_1[i]);
        }
    }
    runCount += i;

    PRT_Printf("Run total testcase %d, failed %d\n", runCount, failCount);
}



