#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <prt_atomic.h>
#include "kern_test_public.h"

#if defined(USE_P_MUTEX)
#include "pthread.h"
#endif

// 共享资源和自旋锁
volatile int a_counter = 0;
int test_time = 60 * 1 * 1;

// 任务 A 的入口函数
void *task_a(void *arg) {
  while (1)
  {
    // 每个系统 tick 自加 1
    a_counter++;
    // 增加延时，确保 task_b 有机会获取锁
    PRT_TaskDelay(1); // 增加延时
    // PRT_Printf("enter function a cycle.\n");
    if(g_testFinish){
        PRT_Printf("exit task a.\n");
        break;
    }
  }
  return NULL;
}

struct PrtSpinLock spinLock;
// 任务 B 的入口函数
void *task_b(void *arg) {
  int counter_read1, counter_read2;
  uintptr_t intSave;
  

  while (1) {
    PRT_Printf("enter function b.\n");
    intSave = PRT_SplIrqLock(&spinLock);

    // 读取计数器
    counter_read1 = a_counter;

    // 延时1个tick以上
    PRT_ClkDelayMs(2000);

    // 再次读取计数器
    counter_read2 = a_counter;

    PRT_SplIrqUnlock(&spinLock, intSave);

    // 检查两次计数器是否一致
    if (counter_read1 != counter_read2) {
      PRT_Printf("Error: A counter inconsistency detected! %d != %d\n", counter_read1, counter_read2);
      g_testResult = 1;
      break;
    }
    
    if(g_testFinish){
        PRT_Printf("exit task b.\n");
        break;
    }

    PRT_TaskDelay(15000); // 增加延时
    PRT_Printf("exec function b end.\n");
  }

  return NULL;
}

int test_spinlock() {

  PRT_Printf("SPINLOCK Demo \n");
  g_testResult = 0;
  g_testFinish = 0;

  pthread_t thread_a, thread_b;
  pthread_attr_t attr_a, attr_b;
  struct sched_param param_a, param_b;

  // 初始化线程属性
  pthread_attr_init(&attr_a);
  pthread_attr_init(&attr_b);

  // 设置线程 A 的优先级和调度策略
  param_a.sched_priority = 254; // 设置优先级 A (值越高，优先级越高)
  pthread_attr_setschedpolicy(&attr_a, SCHED_FIFO); // 设置调度策略
  pthread_attr_setschedparam(&attr_a, &param_a); // 设置优先级

  // 设置线程 B 的优先级和调度策略
  param_b.sched_priority = 100; // 设置优先级 B (值比 A 低)
  pthread_attr_setschedpolicy(&attr_b, SCHED_FIFO); // 设置调度策略
  pthread_attr_setschedparam(&attr_b, &param_b); // 设置优先级

  // 创建任务 A 和任务 B
  pthread_create(&thread_a, &attr_a, task_a, NULL);
  pthread_create(&thread_b, &attr_b, task_b, NULL);

  while (test_time--)
  {
    sleep(1);
  }
  g_testFinish = 1;

  if(g_testResult){
    PRT_Printf("test fail, please check.\n");
  }
  
  return g_testResult;

}

test_case_t g_cases[] = {
    TEST_CASE_Y(test_spinlock),
    // 不需要支持
    // TEST_CASE_Y(test_priority_inherit),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    printf("Spinlock test finished\n");
}