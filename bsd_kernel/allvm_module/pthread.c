//===-- pthread.c ---------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// pthread stubs
//
//===----------------------------------------------------------------------===//

#include "stubs.h"

UNSUPPORTED(pthread_attr_destroy);
UNSUPPORTED(pthread_attr_init);
UNSUPPORTED(pthread_attr_setstacksize);
UNSUPPORTED(pthread_create);
UNSUPPORTED(pthread_getspecific);
UNSUPPORTED(pthread_join);
UNSUPPORTED(pthread_key_create);
UNSUPPORTED(pthread_key_delete);
UNSUPPORTED(pthread_mutex_destroy);
UNSUPPORTED(pthread_mutex_init);
UNSUPPORTED(pthread_mutex_lock);
UNSUPPORTED(pthread_mutex_trylock);
UNSUPPORTED(pthread_mutex_unlock);
UNSUPPORTED(pthread_mutexattr_destroy);
UNSUPPORTED(pthread_mutexattr_init);
UNSUPPORTED(pthread_mutexattr_settype);
UNSUPPORTED(pthread_once);
UNSUPPORTED(pthread_rwlock_destroy);
UNSUPPORTED(pthread_rwlock_init);
UNSUPPORTED(pthread_rwlock_rdlock);
UNSUPPORTED(pthread_rwlock_unlock);
UNSUPPORTED(pthread_rwlock_wrlock);
UNSUPPORTED(pthread_setspecific);
