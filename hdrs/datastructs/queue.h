// This file is part of keepcoding_core
// ==================================
//
// queue.h
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * The Queue struct is a modified version of the List that enforces
 * specific rules regarding the addition and retrieval of items. In this
 * structure, items are consistently added to the end of the queue and removed
 * from the front, following the FIFO (first-in first-out) concept. The methods
 * implemented in the Queue struct primarily make use of the corresponding
 * methods in List in a predefined manner.
 *
 * To create and destroy instances of the List struct, it is recommended
 * to use the constructor and destructor functions.
 *
 * It's important to note that when using member functions, a reference to the
 * Queue instance needs to be passed, similar to how "self" is passed to
 * class member functions in Python. This allows for accessing and manipulating
 * the Queue object's data and behavior.
 */

#ifndef KC_QUEUE_H
#define KC_QUEUE_H

#include "../logger/console_log.h"
#include "list.h"

#include <stdio.h>

struct Queue
{
  struct kc_list_t* list;

  struct kc_console_log_t* log;

  size_t (*length)  (struct Queue* self);
  void*  (*peek)    (struct Queue* self);
  void   (*pop)     (struct Queue* self);
  void   (*push)    (struct Queue* self, void* data, size_t size);
};

struct Queue* new_queue      ();
void          destroy_queue  (struct Queue* queue);

#endif /* KC_QUEUE_H */
