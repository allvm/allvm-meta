/* Horizon bytecode compiler
 * Copyright (C) 2010 James Molloy
 *
 * Horizon is open source software, released under the terms of the Non-Profit
 * Open Software License 3.0. You should have received a copy of the
 * licensing information along with the source code distribution. If you
 * have not received a copy of the license, please refer to the Horizon
 * project website.
 *
 * Please note that if you modify this file, the license requires you to
 * ADD your name to the list of contributors. This boilerplate is not the
 * license itself; please refer to the copy of the license you have received
 * for complete terms.
 */

/**@file   pthreadStubs.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Tue Aug 24 09:57:18 2010
 * @brief  Contains stub routines for pthread_*() functions. */


#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/assert.h>

#include <pthread.h>

#define NUM_KEYS 64
static uint8_t s_key_available[NUM_KEYS];
static void *s_key_value[NUM_KEYS];

typedef void (*_dty)(void*);
static _dty s_key_destructor[NUM_KEYS];

extern "C" int pthread_mutex_lock(pthread_mutex_t *mutex) {
	return 0;
}

extern "C" int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	return 0;
}

extern "C" int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	return 0;
}

extern "C" int pthread_mutex_init(pthread_mutex_t * mutex,
				  const pthread_mutexattr_t * attr) {
	return 0;
}

extern "C" int pthread_mutex_destroy(pthread_mutex_t *mutex) {
	return 0;
}

extern "C" int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
	return 0;
}

extern "C" int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
	return 0;
}

extern "C" int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr,
					    int pshared) {
	return 0;
}

extern "C" int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
	return 0;
}

extern "C" int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr) {
	return 0;
}

extern "C" int pthread_rwlockattr_init(pthread_rwlockattr_t *attr) {
	return 0;
}

extern "C" int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr,
					     int pshared) {
	return 0;
}

extern "C" int pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
	return 0;
}

extern "C" int pthread_rwlock_init(pthread_rwlock_t * rwlock,
				   const pthread_rwlockattr_t * attr) {
	return 0;
}

extern "C" int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
	return 0;
}

extern "C" int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
	return 0;
}

extern "C" int pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
	return 0;
}

extern "C" int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) {
	/* Get a new key. */
	int i = -1;
	for(int j = 0; j < NUM_KEYS; j++) {
		if(s_key_available[j]) {
			i = j;
			break;
		}
	}

	EarlyAssert(i != -1);

	s_key_available[i] = 0;
	s_key_value[i] = 0;
	s_key_destructor[i] = destructor;

	*key = (pthread_key_t)i;

	return 0;
}

extern "C" int pthread_setspecific(pthread_key_t key, const void *value) {
	int i = (int)key;

	EarlyAssert(s_key_available[i] == 0);
	s_key_value[i] = (void*)value;

	return 0;
}

extern "C" void *pthread_getspecific(pthread_key_t key) {
	int i = (int)key;

	EarlyAssert(s_key_available[i] == 0);
	return s_key_value[i];
}

extern "C" int pthread_key_delete(pthread_key_t key) {
	int i = (int)key;

	EarlyAssert(s_key_available[i] == 0);
	void *v = s_key_value[i];

	if(s_key_destructor[i]) {
		s_key_destructor[i](v);
	}
	
	s_key_available[i] = 1;
	return 0;
}

void InitialisePthreads() {
	for(int i = 0; i < NUM_KEYS; i++) {
		s_key_available[i] = 1;
	}
}
