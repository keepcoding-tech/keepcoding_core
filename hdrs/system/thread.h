// This file is part of keepcoding_core
// ==================================
//
// thread.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a thread pool
 */

#ifndef KC_THREAD_T_H
#define KC_THREAD_T_H

#include <windows.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

struct kc_thread_t
{
  HANDLE _abort;
  HANDLE _thread;
  HANDLE _start;
  HANDLE _mutex;

  unsigned int _timeout;

  int (*start)       (struct kc_thread_t* self, int priority, unsigned int timeout);
  int (*stop)        (struct kc_thread_t* self, unsigned int timeout);
  int (*wait)        (struct kc_thread_t* self, int count, HANDLE* event, unsigned int timeout);
  int (*is_pending)  (struct kc_thread_t* self, bool* pending);
  int (*on_event)    (struct kc_thread_t* self, int event);
};

struct kc_thread_t* new_thread      ();
void                destroy_thread  (struct kc_thread_t* thread);

static unsigned int (*callback)  (void* param);

//---------------------------------------------------------------------------//

#endif /* KC_THREAD_T_H */
