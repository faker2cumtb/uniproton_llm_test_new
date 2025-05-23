/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-25     huajian      the first version
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <prt_sys_external.h>
#include <prt_sem.h>

#define LOOPS 50000
#define NTHREADS 2

// 定义互斥锁
pthread_mutex_t mutex;


void *thread_func(void *arg) {
    S64 start, end, thread_total_time;

    for (int i = 0; i < LOOPS; ++i) {
        // 尝试获取互斥锁
        start = PRT_ClkGetCycleCount64();
        pthread_mutex_lock(&mutex);
        end = PRT_ClkGetCycleCount64();
        thread_total_time += PRT_ClkCycle2Ns(end - start);

        // 模拟一些工作负载,确保在锁获取释放过程中存在任务调度切换
        // PRT_TaskDelay(1); 

        // 释放互斥锁
        start = PRT_ClkGetCycleCount64();
        pthread_mutex_unlock(&mutex);
        end = PRT_ClkGetCycleCount64();
        thread_total_time += PRT_ClkCycle2Ns(end - start);
    }

	 // 计算平均时间（单位：微秒）
    double average_time_us = thread_total_time / (LOOPS * 1000.0);

    printf("Average time per lock/unlock operation: %.6f μs\n", average_time_us);

    pthread_exit(NULL);
}

int release_mutex_test() {
    pthread_t threads[NTHREADS];
    int ids[NTHREADS];
    struct timespec start, end;
    long total_time = 0;

    // 初始化互斥锁
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Mutex initialization failed\n");
        return 1;
    }

    // 创建线程
    for (int i = 0; i < NTHREADS; ++i) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &ids[i]) != 0) {
            printf(stderr, "Thread creation failed\n");
            return 1;
        }
    }

    // 等待所有线程完成
    for (int i = 0; i < NTHREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // 清理互斥锁
    pthread_mutex_destroy(&mutex);

    // 检查是否满足条件
    return 0;
}