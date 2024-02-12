// This file is part of keepcoding_core
// ==================================
//
// list.c
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/datastructs/list.h"
#include "../../hdrs/common.h"

#include <stdlib.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int erase_all_nodes       (struct kc_list_t* self);
static int erase_first_node      (struct kc_list_t* self);
static int erase_last_node       (struct kc_list_t* self);
static int erase_node            (struct kc_list_t* self, int index);
static int erase_nodes_by_value  (struct kc_list_t* self, void* value, int (*compare)(const void* a, const void* b));
static int get_first_node        (struct kc_list_t* self, struct kc_node_t* front_node);
static int get_last_node         (struct kc_list_t* self, struct kc_node_t* back_node);
static int get_node              (struct kc_list_t* self, int index, struct kc_node_t* node);
static int insert_new_head       (struct kc_list_t* self, void* data, size_t size);
static int insert_new_node       (struct kc_list_t* self, int index, void* data, size_t size);
static int insert_new_tail       (struct kc_list_t* self, void* data, size_t size);
static int is_list_empty         (struct kc_list_t* self, bool* is_empty);
static int search_node           (struct kc_list_t* self, void* value, int (*compare)(const void* a, const void* b), bool* exists);

static struct kc_node_t* iterate_ll           (struct kc_list_t* list, int index);
static struct kc_node_t* iterate_forward_ll   (struct kc_node_t* head, int index);
static struct kc_node_t* iterate_reverse_ll   (struct kc_node_t* tail, int index);

//---------------------------------------------------------------------------//

struct kc_list_t* new_list()
{
  // create a List instance to be returned
  struct kc_list_t* new_list = malloc(sizeof(struct kc_list_t));

  // confirm that there is memory to allocate
  if (new_list == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // create a console log instance to be used for the list
  struct kc_console_log_t* logger = new_console_log(err, log_err, __FILE__);

  // confirm that there is memory to allocate
  if (logger == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the list instance
    free(new_list);

    return NULL;
  }

  // initialize the structure members fields
  new_list->head   = NULL;
  new_list->tail   = NULL;
  new_list->length = 0;
  new_list->log    = logger;

  // assigns the public member methods
  new_list->back       = get_last_node;
  new_list->clear      = erase_all_nodes;
  new_list->empty      = is_list_empty;
  new_list->erase      = erase_node;
  new_list->front      = get_first_node;
  new_list->get        = get_node;
  new_list->insert     = insert_new_node;
  new_list->pop_back   = erase_last_node;
  new_list->pop_front  = erase_first_node;
  new_list->push_back  = insert_new_tail;
  new_list->push_front = insert_new_head;
  new_list->remove     = erase_nodes_by_value;
  new_list->search     = search_node;

  return new_list;
}

//---------------------------------------------------------------------------//

void destroy_list(struct kc_list_t* list)
{
  // if the list reference is NULL, abort the application
  if (list == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  erase_all_nodes(list);
  free(list);
}

//---------------------------------------------------------------------------//

int erase_all_nodes(struct kc_list_t* self)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // start iterating from the head
  struct kc_node_t* cursor = self->head;
  while (cursor != NULL)
  {
    struct kc_node_t* next = cursor->next;
    node_destructor(cursor);
    cursor = next;
  }

  // reset the head, tail and size
  self->head = NULL;
  self->tail = NULL;
  self->length = 0;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_first_node(struct kc_list_t* self)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  struct kc_node_t* old_head = self->head;

  // check if this is alos the last node
  if (old_head->next == NULL)
  {
    self->head = NULL;
    self->tail = NULL;
  }
  else
  {
    self->head = old_head->next;
    self->head->prev = NULL;
  }

  node_destructor(old_head);
  --self->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_last_node(struct kc_list_t* self)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  struct kc_node_t* old_tail = self->tail;

  // check if this is alos the last node
  if (old_tail->prev == NULL)
  {
    self->tail = NULL;
    self->head = NULL;
  }
  else
  {
    self->tail = old_tail->prev;
    self->tail->next = NULL;
  }

  node_destructor(old_tail);
  --self->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_node(struct kc_list_t* self, int index)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index >= self->length)
  {
    // log the warning to the console
    self->log->warning(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INVALID;
  }

  // check if the item being removed is the head
  if (index == 0)
  {
    return erase_first_node(self);
  }

  // check if the item being removed is the tail
  if (index == self->length - 1)
  {
    return erase_last_node(self);
  }

  // find the node in the list before the one that is going to be removed
  struct kc_node_t* current = iterate_ll(self, index - 1);

  // use the node returned to define the node to be removed
  struct kc_node_t *node_to_remove = current->next;
  current->next = node_to_remove->next;
  current->next->prev = current;

  node_destructor(node_to_remove);

  --self->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_nodes_by_value(struct kc_list_t* self, void* value, int (*compare)(const void* a, const void* b))
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // start from the head
  struct kc_node_t* cursor = self->head;
  size_t index = 0;

  // search the node by value
  while (cursor != NULL)
  {
    if (compare(cursor->data, value) == 0)
    {
      // erase the head
      if (index == 0)
      {
        cursor = cursor->next;

        int rez = erase_first_node(self);
        if (rez != KC_SUCCESS)
        {
          return rez;
        }

        continue;
      }

      // erase the tail
      if (index == self->length - 1)
      {
        int rez = erase_last_node(self);
        if (rez != KC_SUCCESS)
        {
          return rez;
        }

        break;
      }

      // use the node cursor to define the node to be removed
      struct kc_node_t *node_to_remove = cursor;
      cursor->prev->next = cursor->next;
      cursor->next->prev = cursor->prev;
      cursor = cursor->next;

      node_destructor(node_to_remove);
      --self->length;
      continue;
    }

    // continue searching
    cursor = cursor->next;
    ++index;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_first_node(struct kc_list_t* self, struct kc_node_t* first_node)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  first_node = self->head;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_last_node(struct kc_list_t* self, struct kc_node_t* last_node)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  last_node = self->tail;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_node(struct kc_list_t* self, int index, struct kc_node_t* node)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index >= self->length)
  {
    // log the warning to the console
    self->log->warning(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INDEX_OUT_OF_BOUNDS;
  }

  node = iterate_ll(self, index);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int insert_new_head(struct kc_list_t* self, void* data, size_t size)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  return insert_new_node(self, 0, data, size);
}

//---------------------------------------------------------------------------//

int insert_new_node(struct kc_list_t* self, int index, void* data, size_t size)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index > self->length)
  {
    // log the warning to the console
    self->log->warning(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INVALID;
  }

  // create a new node to be inserted
  struct kc_node_t* new_node = node_constructor(data, size);

  // if the node is NULL, don't make the insertion
  if (new_node == NULL)
  {
    return KC_INVALID; /* an error has already been displayed */
  }

  // check if this node will be the new head
  if (index == 0)
  {
    new_node->next = self->head;
    self->head = new_node;

    // if length is less than 1 then head is also tail
    if (self->length == 0)
    {
      self->tail = new_node;
    }

    ++self->length;

    return KC_SUCCESS;
  }

  // find the item in the list immediately before the desired index
  struct kc_node_t* cursor = iterate_ll(self, index - 1);

  if (cursor == NULL)
  {
    return KC_INVALID; /* an error has already been displayed */
  }

  new_node->next = cursor->next;
  new_node->prev = cursor;
  cursor->next = new_node;

  // the "prev" of the third node must point to the new node
  if (new_node->next)
  {
    new_node->next->prev = new_node;
  }

  // check if this node will be the new tail
  if (index == self->length)
  {
    self->tail = new_node;
  }

  // increment the list length
  ++self->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int insert_new_tail(struct kc_list_t* self, void* data, size_t size)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  return insert_new_node(self, (int)self->length, data, size);
}

//---------------------------------------------------------------------------//

int is_list_empty(struct kc_list_t* self, bool* is_empty)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  if (self->length == 0 && self->head == NULL && self->tail == NULL)
  {
    (*is_empty) = true;
  }
  else
  {
    (*is_empty) = false;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int search_node(struct kc_list_t* self, void* value,
    int (*compare)(const void* a, const void* b), bool* exists)
{
  // if the list reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  // create a new node instance
  struct kc_node_t* node = self->head;

  // search the node by value
  while (node != NULL && compare(node->data, value) != 0)
  {
    node = node->next;
  }

  if (node != NULL)
  {
    (*exists) = true;
  }
  else
  {
    (*exists) = false;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

struct kc_node_t* iterate_ll(struct kc_list_t* self, int index)
{
  // if the list reference is NULL, do nothing
  if (self == NULL || self->head == NULL || self->tail == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index >= self->length)
  {
    // log the warning to the console
    self->log->warning(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return NULL;
  }

  // check if the index is over the half of the list length, if the index is
  // smaller, then start from the head, otherwise start from the tail
  struct kc_node_t* node = index <= self->length / 2 ?
      iterate_forward_ll(self->head, index) :
      iterate_reverse_ll(self->tail, (int)(self->length - 1) - index);

  return node;
}

//---------------------------------------------------------------------------//

struct kc_node_t* iterate_forward_ll(struct kc_node_t* head, int index)
{
  struct kc_node_t* cursor = head;
  for (int i = 0; i < index; ++i)
  {
    cursor = cursor->next;
  }
  return cursor;
}

//---------------------------------------------------------------------------//

struct kc_node_t* iterate_reverse_ll(struct kc_node_t* tail, int index)
{
  struct kc_node_t* cursor = tail;
  for (int i = 0; i < index; ++i)
  {
    cursor = cursor->prev;
  }
  return cursor;
}

//---------------------------------------------------------------------------//

