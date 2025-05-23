/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that when a pthread_mutex_destroy is called on a 
 *  locked mutex, it fails and returns EBUSY

 * Steps:
 * 1. Create a mutex 
 * 2. Lock the mutex
 * 3. Try to destroy the mutex
 * 4. Check that this may fail with EBUSY
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "4-2"
#define FUNCTION "pthread_mutex_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int pthread_mutex_destroy_4_2()
{
	pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
	int                 rc = 0;

	/* Lock the mutex */
	rc = pthread_mutex_lock(&mutex);
	if(rc != 0) {
		perror(ERROR_PREFIX "pthread_mutex_lock\n");
		return PTS_UNRESOLVED ;
	}

	/* Try to destroy the locked mutex */
	rc = pthread_mutex_destroy(&mutex);
	if(rc == 0)
	{
		printf(ERROR_PREFIX "Test FAIL: Expected return error\n");
		return PTS_FAIL ;
	}else
	{
		if(rc != EBUSY)
		{
			printf(ERROR_PREFIX "Test PASS: Expected %d(EBUSY) got %d, "
				"though the standard states 'may' fail\n", EBUSY, rc);
		}
		
	}
	
	rc = pthread_mutex_unlock(&mutex);
	if(rc != 0) {
		perror(ERROR_PREFIX "pthread_mutex_lock\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_mutex_destroy(&mutex);
	if(rc != 0) {
		perror(ERROR_PREFIX "pthread_mutex_destroy\n");
		return PTS_UNRESOLVED ;
	}
		
	printf("Test PASS\n");
	return PTS_PASS ;
}
