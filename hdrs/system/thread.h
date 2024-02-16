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

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

//---------------------------------------------------------------------------//

struct kc_thread_t
{
  pthread_t _thread;
  unsigned int _timeout;

  int (*start)       (struct kc_thread_t* self, void* (*thread_func)(void* arg));
  int (*stop)        (struct kc_thread_t* self);
  int (*wait)        (struct kc_thread_t* self);
  int (*is_pending)  (struct kc_thread_t* self, bool* pending);
  int (*on_event)    (struct kc_thread_t* self, int* event);
};

struct kc_thread_t* new_thread      ();
void                destroy_thread  (struct kc_thread_t* thread);

// static unsigned int (*callback)  (void* param);

//---------------------------------------------------------------------------//

#endif /* KC_THREAD_T_H */
