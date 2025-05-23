#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int hwi_entry_hook(void);
extern int hwi_exit_hook(void);
extern int idle_before_hook_test(void);
extern int idle_hook_test(void);
extern int task_del_hook(void);
extern int task_error_hook(void);
extern int task_create_hook(void);
extern int task_switch_hook(void);

extern U32 hook_callback(TskHandle taskPid);
extern void task_demo(void);
extern void timer_demo(void);
extern void print_time(void);
extern U32 hwi_hook_callback(U32 hwiNum);
extern time_t raw_time1, raw_time2, raw_time3;
extern int hook_count;

#endif

#define PTS_PASS        0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5