// This file is part of keepcoding_core
// ==================================
//
// tree.h
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * The Binary Tree structure is made up of Nodes, which hold references to
 * at most two children. The left child contains data values that are smaller
 * than the parent, while the right child contains values greater than the
 * parent.
 *
 * When creating a Tree, users need to define their own comparison
 * function for the tree. It's important to note that these functions should
 * compare the data stored in the Nodes, not the Nodes themselves.
 *
 * To make things easier, there is a generic comparison function available
 * for users to utilize by important the COMPARE_TREE macro.
 *
 * To create and destroy instances of the Tree struct, it is recommended
 * to use the constructor and destructor functions.
 *
 * It's important to note that when using member functions, a reference to the
 * Tree instance needs to be passed, similar to how "self" is passed to
 * class member functions in Python. This allows for accessing and manipulating
 * the Tree object's data and behavior.
 */

#ifndef KC_TREE_H
#define KC_TREE_H

#include "../logger/console_log.h"
#include "node.h"

#include <stdio.h>

struct Tree
{
  struct kc_node_t* root;

  struct kc_console_log_t* log;

  int          (*compare)  (const void* a, const void* b);
  void         (*insert)   (struct Tree* self, void* data, size_t size);
  void         (*remove)   (struct Tree* self, void* data, size_t size);
  struct kc_node_t* (*search)   (struct Tree* self, void* data);
};

//---------------------------------------------------------------------------//

struct Tree* new_tree      (int (*compare)(const void* a, const void* b));
void         destroy_tree  (struct Tree* tree);

//---------------------------------------------------------------------------//

// use this macro to define any type of primitive data comparison function
#define COMPARE_TREE(type, function_name)           \
  int function_name(const void* a, const void* b)   \
  {                                                 \
    if (*(type*)a < *(type*)b)                      \
    {                                               \
      return -1;                                    \
    }                                               \
    if (*(type*)a > *(type*)b)                      \
    {                                               \
      return 1;                                     \
    }                                               \
    return 0;                                       \
  }

#endif /* KC_TREE_H */
