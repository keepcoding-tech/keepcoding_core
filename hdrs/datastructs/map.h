// This file is part of keepcoding_core
// ==================================
//
// map.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a hash table struct
 */

#ifndef KC_MAP_T_H
#define KC_MAP_T_H

#include <stdio.h>

//---------------------------------------------------------------------------//

#define KC_SERVER_LOG_PATH "build/log/server.log"

#define KC_MAP_MAX_SIZE 128

//---------------------------------------------------------------------------//

struct kc_entry_t
{
  char* key;
  void* val;

  struct kc_entry_t* next;
};

struct kc_entry_t* new_entry      (const char* key, void* val, size_t val_size);
void               destroy_entry  (struct kc_entry_t* entry);

//---------------------------------------------------------------------------//

struct kc_map_t
{
  struct kc_entry_t** entries;

  int (*set)  (struct kc_map_t* self, const char* key, void* val, size_t val_size);
  int (*get)  (struct kc_map_t* self, const char* key, void** val);
};

struct kc_map_t* new_map      (void);
void             destroy_map  (struct kc_map_t* map);

//---------------------------------------------------------------------------//

#endif /* KC_MAP_T_H */
