// This file is part of keepcoding_core
// ==================================
//
// vector.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/common.h"
#include "../../hdrs/datastructs/vector.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int erase_all_elems         (struct kc_vector_t* self);
static int erase_elem              (struct kc_vector_t* self, int index);
static int erase_elems_by_value    (struct kc_vector_t* self, void* value, int (*compare)(const void* a, const void* b));
static int erase_first_elem        (struct kc_vector_t* self);
static int erase_last_elem         (struct kc_vector_t* self);
static int get_elem                (struct kc_vector_t* self, int index, void** at);
static int get_first_elem          (struct kc_vector_t* self, void** front);
static int get_last_elem           (struct kc_vector_t* self, void** back);
static int get_vector_capacity     (struct kc_vector_t* self, size_t* max_size);
static int insert_at_beginning     (struct kc_vector_t* self, void* data, size_t size);
static int insert_at_end           (struct kc_vector_t* self, void* data, size_t size);
static int is_vector_empty         (struct kc_vector_t* self, bool* empty);
static int insert_new_elem         (struct kc_vector_t* self, int index, void* data, size_t size);
static int resize_vector_capacity  (struct kc_vector_t* self, size_t new_capacity);
static int search_elem             (struct kc_vector_t* self, void* value, int (*compare)(const void* a, const void* b), bool* exists);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static void permute_to_left         (struct kc_vector_t* vector, int start, int end);
static void permute_to_right        (struct kc_vector_t* vector, int start, int end);
static void resize_vector           (struct kc_vector_t* vector, size_t new_capacity);

//---------------------------------------------------------------------------//

struct kc_vector_t* new_vector()
{
  // create a Vector instance to be returned
  struct kc_vector_t* new_vector = malloc(sizeof(struct kc_vector_t));

  // confirm that there is memory to allocate
  if (new_vector == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  new_vector->log = new_logger(err, log_err, __FILE__);

  if (new_vector == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    free(new_vector);

    return NULL;
  }

  // initialize the structure members fields
  new_vector->capacity = 16;
  new_vector->length   = 0;
  new_vector->data     = malloc(16 * sizeof(void*));

  // confirm that there is memory to allocate
  if (new_vector->data == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the instances and exit
    free(new_vector->log);
    free(new_vector->data);
    free(new_vector);

    return NULL;
  }

  // assigns the public member methods
  new_vector->at         = get_elem;
  new_vector->back       = get_last_elem;
  new_vector->clear      = erase_all_elems;
  new_vector->empty      = is_vector_empty;
  new_vector->erase      = erase_elem;
  new_vector->front      = get_first_elem;
  new_vector->insert     = insert_new_elem;
  new_vector->max_size   = get_vector_capacity;
  new_vector->pop_back   = erase_last_elem;
  new_vector->pop_front  = erase_first_elem;
  new_vector->push_back  = insert_at_end;
  new_vector->push_front = insert_at_beginning;
  new_vector->remove     = erase_elems_by_value;
  new_vector->resize     = resize_vector_capacity;
  new_vector->search     = search_elem;

  return new_vector;
}

//---------------------------------------------------------------------------//

void destroy_vector(struct kc_vector_t* vector)
{
  // if the vector reference is NULL, do nothing
  if (vector == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  destroy_logger(vector->log);

  // free the memory for each element and the array itself
  if (vector->data != NULL)
  {
    for (int i = 0; i < vector->length; ++i)
    {
      if (vector->data[i] != NULL)
      {
        free(vector->data[i]);
      }
    }
  }

  free(vector->data);
  free(vector);
}

//---------------------------------------------------------------------------//

int erase_all_elems(struct kc_vector_t* self)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // free the memory for each element
  if (self->data != NULL)
  {
    for (int i = 0; i < self->length; ++i)
    {
      if (self->data[i] != NULL)
      {
        free(self->data[i]);
      }
    }
  }

  // reallocate the default capacity
  if (self->capacity > 16)
  {
    resize_vector(self, 16);
  }

  // reset the length
  self->length = 0;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_elem(struct kc_vector_t* self, int index)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // make sure the list is not empty
  if (self->length == 0)
  {
    self->log->error(self->log, KC_EMPTY_STRUCTURE, __LINE__, __func__);

    return KC_EMPTY_STRUCTURE;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index >= self->length)
  {
    self->log->error(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INDEX_OUT_OF_BOUNDS;
  }

  // free the memory from the desired position
  permute_to_left(self, index, self->length);
  --self->length;

  // resize if the length of the vector is less than half
  if (self->length < self->capacity / 2 && self->capacity > 16)
  {
    resize_vector(self, self->capacity / 2);
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_elems_by_value(struct kc_vector_t* self, void* value,
    int (*compare)(const void* a, const void* b))
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // go through the array and check each element
  int index = 0;
  while (index < self->length)
  {
    if (compare(self->data[index], value) == 0)
    {
      int rez = erase_elem(self, index);
      if (rez != KC_SUCCESS)
      {
        return rez;
      }

      continue;
    }
    ++index;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_first_elem(struct kc_vector_t* self)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = erase_elem(self, 0);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int erase_last_elem(struct kc_vector_t* self)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = erase_elem(self, self->length - 1);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_elem(struct kc_vector_t* self, int index, void** at)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // make sure the list is not empty
  if (self->length == 0)
  {
    self->log->error(self->log, KC_EMPTY_STRUCTURE, __LINE__, __func__);

    return KC_EMPTY_STRUCTURE;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index >= self->length)
  {
    self->log->error(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INDEX_OUT_OF_BOUNDS;
  }

  (*at) = self->data[index];

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_first_elem(struct kc_vector_t* self, void** first)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = get_elem(self, 0, first);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_last_elem(struct kc_vector_t* self, void** back)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = get_elem(self, self->length - 1, back);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_vector_capacity(struct kc_vector_t* self, size_t* max_size)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  (*max_size) = self->capacity;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int insert_at_beginning(struct kc_vector_t* self, void* data, size_t size) {
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = insert_new_elem(self, 0, data, size);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int insert_at_end(struct kc_vector_t* self, void* data, size_t size)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  int rez = insert_new_elem(self, self->length, data, size);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int is_vector_empty(struct kc_vector_t* self, bool* empty) {
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  if (self->length == 0)
  {
    (*empty) = true;
  }
  else
  {
    (*empty) = false;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int insert_new_elem(struct kc_vector_t* self, int index, void* data, size_t size)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // confirm the user has specified a valid index
  if (index < 0 || index > self->length)
  {
    self->log->error(self->log, KC_INDEX_OUT_OF_BOUNDS, __LINE__, __func__);

    return KC_INDEX_OUT_OF_BOUNDS;
  }

  // reallocate more memory if the capacity is full
  if (self->length + 1 >= self->capacity)
  {
    resize_vector(self, self->capacity * 2);
  }

  // alocate space in memory
  void* new_elem = malloc(size);

  // check if the memory allocation was succesfull
  if (new_elem == NULL)
  {
    self->log->error(self->log, KC_OUT_OF_MEMORY, __LINE__, __func__);

    return KC_OUT_OF_MEMORY;
  }

  // insert the value at the specified location
  memcpy(new_elem, data, size);
  permute_to_right(self, index, self->length);
  self->data[index] = new_elem;
  ++self->length;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int resize_vector_capacity(struct kc_vector_t* self, size_t new_capacity)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  resize_vector(self, new_capacity);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int search_elem(struct kc_vector_t* self, void* value,
    int (*compare)(const void* a, const void* b), bool* exists)
{
  // if the vector reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // go through the array and return true if found
  for (int i = 0; i < self->length; ++i)
  {
    if (compare(self->data[i], value) == 0)
    {
      (*exists) = true;

      return KC_SUCCESS;
    }
  }

  (*exists) = false;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void permute_to_left(struct kc_vector_t* vector, int start, int end)
{
  for (int i = start; i < end && i < vector->length; ++i)
  {
    vector->data[i] = vector->data[i + 1];
  }
}

//---------------------------------------------------------------------------//

void permute_to_right(struct kc_vector_t* vector, int start, int end)
{
  for (int i = end; i >= start && i > 0; --i)
  {
    vector->data[i] = vector->data[i - 1];
  }
}

//---------------------------------------------------------------------------//

void resize_vector(struct kc_vector_t* vector, size_t new_capacity)
{
  // make sure the user specific a valid capacity size
  if (new_capacity < 1)
  {
    vector->log->error(vector->log, KC_UNDERFLOW, __LINE__, __func__);
    return;
  }

  // temporarlly store the new data
  void** new_data = realloc(vector->data, new_capacity * sizeof(void*));

  // check if the memory reallocation was succesfull
  if (new_data == NULL)
  {
    vector->log->error(vector->log, KC_OUT_OF_MEMORY, __LINE__, __func__);
    return;
  }

  vector->data = new_data;
  vector->capacity = new_capacity;
}

//---------------------------------------------------------------------------//
