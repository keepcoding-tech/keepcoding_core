// This file is part of keepcoding_core
// ==================================
//
// map.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/datastructs/map.h"
#include "../../hdrs/system/logger.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>

struct kc_entry_t* new_entry(const char* key, void* val, size_t val_size)
{
  // create a new instance to be returned
  struct kc_entry_t* new_entry = malloc(sizeof(struct kc_entry_t));

  // check the alocation of the memory
  if (new_entry == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  new_entry->key = (char*)malloc(sizeof(char) * strlen(key) + 1);
  if (new_entry->key == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the memory first
    free(new_entry);

    return NULL;
  }

  new_entry->val = malloc(val_size);
  if (new_entry->val == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the memory first
    free(new_entry->key);
    free(new_entry);

    return NULL;
  }

  // asign the values
  strcpy(new_entry->key, key);
  memcpy(new_entry->val, val, val_size);

  return new_entry;
}

//---------------------------------------------------------------------------//

void destroy_entry(struct kc_entry_t* entry)
{
  if (entry == NULL)
  {
    return;
  }

  if (entry->key != NULL)
  {
    free(entry->key);
  }

  if (entry->val != NULL)
  {
    free(entry->val);
  }

  free(entry);
}

//---------------------------------------------------------------------------//

int set_map_key  (struct kc_map_t* self, const char* key, void* val, size_t val_size);
int get_map_val  (struct kc_map_t* self, const char* key, void** val);

static unsigned int _hash  (const char* key);

//---------------------------------------------------------------------------//

struct kc_map_t* new_map(void)
{
  // create a new instance to be returned
  struct kc_map_t* new_map = malloc(sizeof(struct kc_map_t));

  // check the alocation of the memory
  if (new_map == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  new_map->entries = malloc(sizeof(struct kc_entry_t) * KC_MAP_MAX_SIZE);
  if (new_map->entries == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the memory first
    free(new_map);

    return NULL;
  }

  // set each entry to NULL
  for (int i = 0; i < KC_MAP_MAX_SIZE; ++i)
  {
    new_map->entries[i] = NULL;
  }

  // asign public function members
  new_map->set = set_map_key;
  new_map->get = get_map_val;

  return new_map;
}

//---------------------------------------------------------------------------//

void destroy_map(struct kc_map_t* map)
{
  if (map == NULL)
  {
    return;
  }

  // TODO: erase all elements

  free(map);
}

//---------------------------------------------------------------------------//

int set_map_key(struct kc_map_t* self, const char* key, void* val, size_t val_size)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (key == NULL || val == NULL)
  {
    return KC_INVALID_ARGUMENT;
  }

  unsigned int slot = _hash(key);

  // try to look up a slot
  struct kc_entry_t* entry = self->entries[slot];

  // no entry means slot empty, insert immediately
  if (entry == NULL)
  {
    self->entries[slot] = new_entry(key, val, val_size);
    return KC_SUCCESS;
  }

  struct kc_entry_t* prev; 

  // go through each entry until the end is 
  // reached or a maching key is found
  while (entry != NULL)
  {
    // check the key for maches
    if (strcmp(entry->key, key) == 0)
    {
      // first, free the value
      free(entry->val);

      // replace the value
      entry->val = malloc(val_size);
      memcpy(entry->val, val, val_size);

      return KC_SUCCESS;
    }

    // go for the next entry
    prev = entry;
    entry = prev->next;
  }

  // if no entries were found, create a new one
  prev->next = new_entry(key, val, val_size);
  
  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_map_val(struct kc_map_t* self, const char* key, void** val)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (key == NULL)
  {
    return KC_INVALID_ARGUMENT;
  }

  unsigned int slot = _hash(key);
  struct kc_entry_t* entry = self->entries[slot];
  
  // no slot means no entry
  if (entry == NULL)
  {
    (*val) = NULL;
    return KC_INVALID;
  }

  // go through each entry in the slot
  while (entry != NULL)
  {
    // check the key
    if (strcmp(entry->key, key) == 0)
    {
      (*val) = entry->val;
      return KC_SUCCESS;
    }

    // proceed to the next entry
    entry = entry->next;
  }

  // the entry was not found
  return KC_INVALID;
}

//---------------------------------------------------------------------------//

static unsigned int _hash(const char* key)
{
  unsigned long int value = 0;
  unsigned int key_len = strlen(key);

  // do several rounds of multipication
  for (int i = 0; i < key_len; ++i)
  {
    value = value * 37 + key[i];
  }

  // make sure the value is 0 <= value <= MAX_SIZE
  value = value % KC_MAP_MAX_SIZE;

  return value;
}

//---------------------------------------------------------------------------//

