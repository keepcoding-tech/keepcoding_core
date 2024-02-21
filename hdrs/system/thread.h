// This file is part of keepcoding_core
// ==================================
//
// thread.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a thread struct
 */

#ifndef KC_THREAD_T_H
#define KC_THREAD_T_H

#include "../logger/logger.h"

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

//---------------------------------------------------------------------------//

struct kc_thread_t
{
  pthread_t _thread;

  int (*start)  (struct kc_thread_t* self, void* (*thread_func)(void* arg), void* arg);
  int (*stop)   (struct kc_thread_t* self);
  int (*join)   (struct kc_thread_t* self, void** value_ptr);
};

//---------------------------------------------------------------------------//

struct kc_thread_t* new_thread      ();
void                destroy_thread  (struct kc_thread_t* thread);

//---------------------------------------------------------------------------//

pthread_mutex_t mutex;

#define kc_mutex_lock           \
  pthread_mutex_lock(&mutex);

#define kc_mutex_unlock         \
  pthread_mutex_unlock(&mutex);

//---------------------------------------------------------------------------//

#endif /* KC_THREAD_T_H */
