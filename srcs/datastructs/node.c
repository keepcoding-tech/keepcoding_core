// This file is part of keepcoding_core
// ==================================
//
// node.c
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/datastructs/node.h"
#include "../../hdrs/common.h"

#include <stdlib.h>

//---------------------------------------------------------------------------//

struct kc_node_t* node_constructor(void* data, size_t size)
{
  if (size < 1) {
    log_error(err[KC_UNDERFLOW], log_err[KC_UNDERFLOW],
      __FILE__, __LINE__, __func__);

    return NULL;
  }

  // create a Node instance to be returned
  // and allocate space for the data
  struct kc_node_t* new_node = malloc(sizeof(struct kc_node_t));

  if (new_node == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
      __FILE__, __LINE__, __func__);

    return NULL;
  }

  new_node->data = malloc(size);

  if (new_node->data == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
      __FILE__, __LINE__, __func__);

    // free the node instances
    free(new_node);

    return NULL;
  }

  // copy the block of memory
  memcpy(new_node->data, data, size);

  // initialize the pointers
  new_node->next = NULL;
  new_node->prev = NULL;

  return new_node;
}

//---------------------------------------------------------------------------//

void node_destructor(struct kc_node_t* node)
{
  // destroy node only if is not dereferenced
  if (node == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return;
  }

  free(node->data);
  free(node);
}

//---------------------------------------------------------------------------//
