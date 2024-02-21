// This file is part of keepcoding_core
// ==================================
//
// thread.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/system/thread.h"
#include "../../hdrs/common.h"

#include <stdlib.h>

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int start_thread      (struct kc_thread_t* self, void* (*thread_func)(void* arg), void* arg);
static int stop_thread       (struct kc_thread_t* self);
static int join_thread       (struct kc_thread_t* self, void** value_ptr);

//---------------------------------------------------------------------------//

struct kc_thread_t* new_thread()
{
  // create a thread instance to be returned
  struct kc_thread_t* new_thread = malloc(sizeof(struct kc_thread_t));

  // confirm that there is memory to allocate
  if (new_thread == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // assigns the public member methods
  new_thread->start = start_thread;
  new_thread->stop  = stop_thread;
  new_thread->join  = join_thread;

  return new_thread;
}

//---------------------------------------------------------------------------//

void destroy_thread(struct kc_thread_t* thread)
{
  if (thread == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  free(thread);
}

//---------------------------------------------------------------------------//

int start_thread(struct kc_thread_t* self, void* (*thread_func)(void* arg), void* arg)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // create the thread and execute the function
  int rez = pthread_create(&(self->_thread), NULL, thread_func, arg);

  if (rez != KC_SUCCESS)
  {
    log_error(err[KC_THREAD_ERROR], log_err[KC_THREAD_ERROR],
        __FILE__, __LINE__, __func__);

    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int stop_thread(struct kc_thread_t* self)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // cancel the thread
  int rez = pthread_cancel(self->_thread);

  if (rez != KC_SUCCESS)
  {
    log_error(err[KC_THREAD_ERROR], log_err[KC_THREAD_ERROR],
        __FILE__, __LINE__, __func__);

    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int join_thread(struct kc_thread_t* self, void** value_ptr)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // join with a terminated thread
  int rez = pthread_join(self->_thread, value_ptr);

  if (rez != KC_SUCCESS)
  {
    log_error(err[KC_THREAD_ERROR], log_err[KC_THREAD_ERROR],
        __FILE__, __LINE__, __func__);

    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
