// This file is part of keepcoding_core
// ==================================
//
// queue.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/datastructs/queue.h"
#include "../../hdrs/common.h"

#include <stdlib.h>

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int get_list_length_queue   (struct kc_queue_t* self, size_t* length);
static int get_next_item_queue     (struct kc_queue_t* self, void** peek);
static int insert_next_item_queue  (struct kc_queue_t* self, void* data, size_t size);
static int remove_next_item_queue  (struct kc_queue_t* self);

//---------------------------------------------------------------------------//

struct kc_queue_t* new_queue()
{
  // create a Queue instance to be returned
  struct kc_queue_t* new_queue = malloc(sizeof(struct kc_queue_t));

  // confirm that there is memory to allocate
  if (new_queue == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // instantiate the queue's List via the constructor
  new_queue->list = new_list();

  if (new_queue->list == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the set instance
    free(new_queue);

    return NULL;
  }

  new_queue->log = new_logger(err, log_err, __FILE__);

  if (new_queue->log == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the set instances
    destroy_list(new_queue->list);
    free(new_queue);

    return NULL;
  }

  // assigns the public member methods
  new_queue->length = get_list_length_queue;
  new_queue->peek   = get_next_item_queue;
  new_queue->pop    = remove_next_item_queue;
  new_queue->push   = insert_next_item_queue;

  return new_queue;
}

//---------------------------------------------------------------------------//

void destroy_queue(struct kc_queue_t* queue)
{
  // if the list reference is NULL, do nothing
  if (queue == NULL)
  {
    // log the warning to the console
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  destroy_logger(queue->log);
  destroy_list(queue->list);
  free(queue);
}

//---------------------------------------------------------------------------//

int get_list_length_queue(struct kc_queue_t* self, size_t* length)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  (*length) = self->list->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_next_item_queue(struct kc_queue_t* self, void** peek)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  struct kc_node_t* next_item = NULL;
  int rez = self->list->front(self->list, &next_item);

  // check if the head of the list exists
  if (next_item != NULL && next_item->data != NULL && rez == KC_SUCCESS)
  {
    (*peek) = next_item->data;

    return KC_SUCCESS;
  }

  return KC_EMPTY_STRUCTURE;
}

//---------------------------------------------------------------------------//

int insert_next_item_queue(struct kc_queue_t *self, void *data, size_t size)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  self->list->push_back(self->list, data, size);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int remove_next_item_queue(struct kc_queue_t *self)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  self->list->pop_front(self->list);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

