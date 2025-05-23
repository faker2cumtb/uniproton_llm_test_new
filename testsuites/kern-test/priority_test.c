#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_log.h"
#include "prt_sys_external.h"
#include "securec.h"
#include "time.h"
#include "kern_test_public.h"

#if defined(USE_P_MUTEX)
#include "pthread.h"
#endif

#define MAX_PRIOR 0
#define MINI_PRIOR 254
#define MIDDLE_PRIOR 128

volatile TskHandle g_task_A = -1;
volatile TskHandle g_task_B = -1;
volatile TskHandle g_task_C = -1;
static int count = 0;

#if defined(USE_P_MUTEX)
static inline U32 test_mutex_init(void *lock)
{
    pthread_mutexattr_t attr = {0};
    pthread_mutexattr_init(&attr);
    attr.type = PTHREAD_MUTEX_RECURSIVE;
    return (U32)pthread_mutex_init((pthread_mutex_t *)lock, &attr);
}

static inline U32 test_mutex_lock(void* lock, U32 timeout)
{
    struct timespec time = {0};
    if (timeout == 0) {
        return (U32)pthread_mutex_trylock((pthread_mutex_t *)lock);
    } else if(timeout == OS_WAIT_FOREVER) {
        return (U32)pthread_mutex_lock((pthread_mutex_t *)lock);
    }
    OsTick2Timeout(&time, timeout);
    return pthread_mutex_timedlock((pthread_mutex_t *)lock, &time);
}

static inline U32 test_mutex_unlock(void* lock)
{
    return pthread_mutex_unlock((pthread_mutex_t *)lock);
}

static U32 test_mutex_destroy(void* lock)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)lock;
    if (mutex->magic == 0) {
        return OS_OK;
    }
    return (U32)pthread_mutex_destroy(mutex);
}
#else
static inline U32 test_mutex_init(void *lock)
{
    return PRT_SemMutexCreate((SemHandle *)lock);
}

static inline U32 test_mutex_lock(void* lock, U32 timeout)
{
    return PRT_SemPend(*(SemHandle *)lock, timeout);
}

static inline U32 test_mutex_unlock(void* lock)
{
    return PRT_SemPost(*(SemHandle *)lock);
}

static U32 test_mutex_destroy(void* lock)
{
    U32 ret;
    if ((*(SemHandle *)lock) != -1) {
        ret = PRT_SemDelete(*(SemHandle *)lock);
        if (ret == OS_OK) {
            *(SemHandle *)lock = -1;
        }
        return ret;
    }
    return OS_OK;
}
#endif

#if defined(USE_P_MUTEX)
pthread_mutex_t g_test_sem_A = {0};
#else
SemHandle g_test_sem_A = -1;
#endif

static void test_task_demo(void)
{
    U32 ret;
    TskHandle self;
    TskPrior selfPrio, result;
    void *lockA = (void *)&g_test_sem_A;
    PRT_TaskSelf(&self);

    /* 等待锁 */
    ret = test_mutex_lock(lockA, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID_FMT(ret, "[ERROR] task %s lock fail", "test_task_demo");

    count++;
    switch (count)
    {
    case 1:
        result = MAX_PRIOR;
        break;
    case 2:
        result = MIDDLE_PRIOR;
        break;
    case 3:
        result = MINI_PRIOR;
        break;
    default:
        result = -1;
        break;
    }

    PRT_TaskGetPriority(self, &selfPrio);
    printf("pri %u task start\n", selfPrio);
    printf("count value: %d, selfPrio value: %u\n", count, result);
    TEST_IF_ERR_RET_VOID((selfPrio != result), "[ERROR] test_task_demo task prior wrong");
    //延时3s
    PRT_TaskDelay(3000);
    
    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task test_256_priority unlock fail");
    printf("check task prior %u success\n", selfPrio);
    if(selfPrio == MINI_PRIOR)
    {
        g_testFinish = 1;
    }
}

/* 255优先级被IDLE程序占用, 不会触发调度，所以使用254优先级进行测试 */
static int test_256_priority(void)
{
    g_testFinish = 0;
    g_testResult = 0;
    U32 ret;
    TskHandle handle;
    printf("start 256_priority_test task\n");
    void *lockA = (void *)&g_test_sem_A;
    ret = test_mutex_init(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lock fail");

    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task test_256_priority lock fail");
    printf("test_256_priority task get mutex lock \n");
    
    handle = test_start_task(test_task_demo, MINI_PRIOR, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] 254 pri task create task fail");

    handle = test_start_task(test_task_demo, MIDDLE_PRIOR, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] 128 pri task create task fail");

    handle = test_start_task(test_task_demo, MAX_PRIOR, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] 0 pri task create task fail");

    //延时3s
    PRT_TaskDelay(3000);
    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task test_256_priority unlock fail");
    while (g_testFinish == 0) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }

    printf("task test_256_priority end\n");
    return g_testResult;
}

static void check_priority(void)
{
    U32 ret;
    TskHandle self;
    TskPrior selfPrio;
    PRT_TaskSelf(&self);
    printf("enter check priority\n");
    PRT_TaskGetPriority(self, &selfPrio);
    printf("selfPrio value: %u\n", selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != MIDDLE_PRIOR), "[ERROR] check_priority task prior wrong");
    printf("check_priority success");
    g_testFinish = 1;
}

static int test_priority_inherit(void)
{
    g_testFinish = 0;
    g_testResult = 0;
    U32 ret;
    TskHandle self, testTskHandle;
    TskPrior selfPrio;
    PRT_TaskSelf(&self);
    ret = PRT_TaskSetPriority(self, MIDDLE_PRIOR);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task set self prior fail");

    PRT_TaskGetPriority(self, &selfPrio);
    printf("test_priority_inherit Prio value: %u\n", selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != MIDDLE_PRIOR), "[ERROR] test_priority_inherit task prior wrong");
    
    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = check_priority;
    param.name = "testinherit";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        printf("create task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return -1;
    }

    while (g_testFinish == 0) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }

    return g_testResult;
}

test_case_t g_cases[] = {
    TEST_CASE_Y(test_256_priority),
    // 不需要支持
    // TEST_CASE_Y(test_priority_inherit),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    printf("priority check test finished\n");
}