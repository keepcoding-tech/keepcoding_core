// This file is part of keepcoding_core
// ==================================
//
// set.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/common.h"
#include "../../hdrs/datastructs/set.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int insert_new_pair_set  (struct kc_set_t* self, void* key, size_t key_size, void* value, size_t value_size);
static int remove_pair_set      (struct kc_set_t* self, void* key, size_t key_size);
static int search_pair_set      (struct kc_set_t* self, void* key, size_t key_size, void** data);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static void recursive_set_destroy  (struct kc_node_t* node);

//---------------------------------------------------------------------------//

struct kc_set_t* new_set(int (*compare)(const void* a, const void* b))
{
  // create a Set instance to be returned
  struct kc_set_t* new_set = malloc(sizeof(struct kc_set_t));

  // confirm that there is memory to allocate
  if (new_set == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // create a console log instance to be used for the set
  new_set->_log = new_logger(err, log_err, __FILE__);

  if (new_set->_log == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the set instance
    free(new_set);

    return NULL;
  }

  // instantiate the set's kc_tree_t via the constructor
  new_set->_entries = new_tree(compare);

  if (new_set->_entries == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    // free the set instances
    destroy_logger(new_set->_log);
    free(new_set);

    return NULL;
  }

  // assigns the public member methods
  new_set->insert = insert_new_pair_set;
  new_set->remove = remove_pair_set;
  new_set->search = search_pair_set;

  return new_set;
}

//---------------------------------------------------------------------------//

void destroy_set(struct kc_set_t* set)
{
  // if the set reference is NULL, do nothing
  if (set == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  // free the binary tree memory
  if (set->_entries->root != NULL)
  {
    recursive_set_destroy(set->_entries->root);
  }

  destroy_logger(set->_log);

  // free the instance too
  free(set);
}

//---------------------------------------------------------------------------//

int insert_new_pair_set(struct kc_set_t* self, void* key,
    size_t key_size, void* value, size_t value_size)
{
  // if the set reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // check if the pair already exists in the set
  struct kc_node_t* node = NULL;
  search_pair_set(self, key, key_size, &node->data);
  if (node != NULL)
  {
    return KC_SUCCESS;
  }

  // create a new Pair
  struct kc_pair_t* pair = pair_constructor(key, key_size, value, value_size);

  // insert that pair into the tree
  int rez = self->_entries->insert(self->_entries, pair, sizeof(struct kc_pair_t));

  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int remove_pair_set(struct kc_set_t* self, void* key, size_t key_size)
{
  // if the set reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // create a new pair by using a dummy value
  char dummy_value = 'a';
  struct kc_pair_t* pair_to_remove = pair_constructor(key, key_size,
      &dummy_value, sizeof(char));

  if (pair_to_remove == NULL)
  {
    return KC_INVALID;
  }

  // call the remove function of the Tree structure
  int rez = self->_entries->remove(self->_entries, pair_to_remove, sizeof(struct kc_pair_t));
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  pair_destructor(pair_to_remove);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int search_pair_set(struct kc_set_t* self, void* key, size_t key_size, void** value)
{
  // if the set reference is NULL, do nothing
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // create a new pair by using a dummy value
  char dummy_value = 'a';
  struct kc_pair_t* searchable = pair_constructor(key, key_size,
      &dummy_value, sizeof(char));

  if (searchable == NULL)
  {
    return KC_INVALID;
  }

  // use the search function of the kc_tree_t to find the desired node
  struct kc_node_t* result_node = NULL;
  int rez = self->_entries->search(self->_entries, searchable, &result_node);
  if (rez != KC_SUCCESS)
  {
    return rez;
  }

  // free the dummy pair
  pair_destructor(searchable);

  // make sure the node was found
  if (result_node != NULL)
  {
    // get the pair from the node
    struct kc_pair_t* result_pair = (struct kc_pair_t*)result_node->data;

    // return either the value for that key or NULL if not found
    if (result_pair != NULL && result_pair->value != NULL)
    {
      (*value) = result_pair->value;

      return KC_SUCCESS;
    }
  }

  return KC_INVALID;
}

//---------------------------------------------------------------------------//

void recursive_set_destroy(struct kc_node_t* node)
{
  // chekc the previous node
  if (node->prev != NULL)
  {
    recursive_set_destroy(node->prev);
  }

  // check the next node
  if (node->next != NULL)
  {
    recursive_set_destroy(node->next);
  }

  // destroy the pair
  pair_destructor(node->data);
}

//---------------------------------------------------------------------------//
