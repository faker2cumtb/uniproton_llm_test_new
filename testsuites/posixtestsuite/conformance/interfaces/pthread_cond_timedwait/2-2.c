/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_timedwait()
 *   shall be equivalent to pthread_cond_wait(), except that an error is returned 
 *   if the absolute time specified by abstime passes before the condition cond is 
 *   signaled or broadcasted.
 * 
 */
 
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define INTERVAL  5
#define TIMEOUT   1

static struct testdata
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
} td;


static int t1_start = 0;

void *pthread_cond_timedwait_2_2_t1_func(void *arg)
{
	int rc;
	struct timeval  curtime;
	struct timespec timeout;
	
	printf("Thread1 started\n");
	
	/* Lock the mutex */
	if (pthread_mutex_lock(&td.mutex) != 0) {
		printf("Thread1 failed to acquire the mutex\n");
		exit(PTS_UNRESOLVED);
	}
	
	/* Tell main it may continue running now that the thread started. */
	t1_start = 1;

	/* Set time for pthread_cond_timedwait to wait */
	if (gettimeofday(&curtime, NULL) !=0 ) {
		printf("Fail to get current time\n");
		exit(PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec + TIMEOUT;
	timeout.tv_nsec = curtime.tv_usec * 1000;
	
	/* Thread will now release the mutex and wait on the condition variable */
	/* The condition variable will not be signaled until AFTER the timeout
	 * period, so we should receive an ETIMEDOUT error. */
	printf("Thread1 is waiting for the cond\n");
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc == ETIMEDOUT) {
		pthread_mutex_unlock(&td.mutex);
		printf("Thread1 stops waiting when time is out\n");
		pthread_exit((void*)PTS_PASS);
	}
	else {
		pthread_mutex_unlock(&td.mutex);
		printf("Test FAILED: pthread_cond_timedwait return %d instead of ETIMEDOUT\n", rc);
		pthread_exit((void*)PTS_FAIL);
        }
}

int pthread_cond_timedwait_2_2()
{
	int rc;
	pthread_t  thread1;
	int th_ret;

	if (pthread_mutex_init(&td.mutex, NULL) != 0) {
		printf("Fail to initialize mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_cond_init(&td.cond, NULL) != 0) {
		printf("Fail to initialize cond\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&thread1, NULL, pthread_cond_timedwait_2_2_t1_func, NULL) != 0) {
		printf("Fail to create thread 1\n");
		return PTS_UNRESOLVED;
	}

	while(!t1_start)	/* wait for thread1 started */
		sleep(1);
	
	/* acquire the mutex released by pthread_cond_wait() within thread 1 */
	if (pthread_mutex_lock(&td.mutex) != 0) {	
		printf("Main failed to acquire mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_mutex_unlock(&td.mutex) != 0) {
		printf("Main failed to release mutex\n");
		return PTS_UNRESOLVED;
	}

	/* Set an alarm signal in case the thread never returns probably because
	 * pthread_cond_timedwait functioned incorrectly and waited too long. */
	// alarm(INTERVAL);

	/* Wait for the thread to return. */
	if(pthread_join(thread1, (void*)&th_ret) != 0)
	{
		printf( "Could not join the thread.\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_mutex_destroy(&td.mutex);
	if(rc != 0) {
		printf("pthread_mutex_destroy failed\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_cond_destroy(&td.cond);
	if(rc != 0) {
		printf("pthread_cond_destroy failed\n");
		return PTS_UNRESOLVED;
	}
	
	if(th_ret == PTS_PASS)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	} else
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
