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

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int start_thread       (struct kc_thread_t* self, void* (*thread_func)(void* arg));
static int stop_thread        (struct kc_thread_t* self);
static int wait_thread        (struct kc_thread_t* self);
static int is_pending_thread  (struct kc_thread_t* self, bool* pending);
static int on_event_thread    (struct kc_thread_t* self, int* event);

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

  new_thread->_timeout = 1;

  new_thread->start      = start_thread;
  new_thread->stop       = stop_thread;
  new_thread->wait       = wait_thread;
  new_thread->is_pending = is_pending_thread;
  new_thread->on_event   = on_event_thread;

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

int start_thread(struct kc_thread_t* self, void* (*thread_func)(void* arg))
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // create the thread and execute the function
  int rez = pthread_create(&(self->_thread), NULL, thread_func, NULL);

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

int wait_thread(struct kc_thread_t* self)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = pthread_join(self->_thread, NULL);

  if (rez != KC_SUCCESS)
  {
    log_error(err[KC_THREAD_ERROR], log_err[KC_THREAD_ERROR],
        __FILE__, __LINE__, __func__);

    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int is_pending_thread(struct kc_thread_t* self, bool* pending)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

    int rez = pthread_self();

    if (rez == KC_SUCCESS)
    {
      (*pending) = false;

      return KC_SUCCESS;
    }
    else if (rez == KC_PENDING)
    {
      (*pending) = true;
      return KC_PENDING;
    }
    else
    {
      return KC_THREAD_ERROR;
    }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int on_event_thread(struct kc_thread_t* self, int* event)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
