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
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  new_thread->_logger = new_logger(KC_THREAD_LOG_PATH);
  if (new_thread->_logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the thread instance
    free(new_thread);

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
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(thread);
}

//---------------------------------------------------------------------------//

int start_thread(struct kc_thread_t* self, void* (*thread_func)(void* arg), void* arg)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create the thread and execute the function
  int ret = pthread_create(&(self->_thread), NULL, thread_func, arg);

  if (ret != KC_SUCCESS)
  {
    log_error(KC_THREAD_ERROR_LOG);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int stop_thread(struct kc_thread_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // cancel the thread
  int ret = pthread_cancel(self->_thread);

  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
        __FILE__, __LINE__, __func__);

    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int join_thread(struct kc_thread_t* self, void** value_ptr)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // join with a terminated thread
  int ret = pthread_join(self->_thread, value_ptr);

  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
        __FILE__, __LINE__, __func__);

    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
