// This file is part of keepcoding_core
// ==================================
//
// file.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * A structure representing file handling functionalities in libkc_system.
 *
 * The File structure encapsulates file interactions, providing methods to
 * manipulate files, retrieve file metadata, and perform file operations within
 * the libkc_system library.
 */

#ifndef KC_FILE_H
#define KC_FILE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//---------------------------------------------------------------------------//

#define KC_FILE_CREATE_NEW                                           0x00000001
#define KC_FILE_CREATE_ALWAYS                                        0x00000002
#define KC_FILE_OPEN_EXISTING                                        0x00000004
#define KC_FILE_OPEN_ALWAYS                                          0x00000008

#define KC_FILE_READ                                                 0x00000010
#define KC_FILE_WRITE                                                0x00000020
#define KC_FILE_DELETE                                               0x00000040
#define KC_FILE_CLOSED                                               0x00000080
#define KC_FILE_DIR_NOT_EMPTY                                        0x00000100

//---------------------------------------------------------------------------//

struct kc_file_t
{
  struct kc_logger_t* log;

  FILE* file;
  char* name;
  char* path;
  int   mode;
  bool  opened;

  int (*close)        (struct kc_file_t* self);
  int (*create_path)  (struct kc_file_t* self, char* path);
  int (*delete)       (struct kc_file_t* self);
  int (*delete_path)  (struct kc_file_t* self, char* path);
  int (*get_mode)     (struct kc_file_t* self, int* mode);
  int (*get_name)     (struct kc_file_t* self, char** name);
  int (*get_path)     (struct kc_file_t* self, char** path);
  int (*is_open)      (struct kc_file_t* self, bool* is_open);
  int (*move)         (struct kc_file_t* self, char* from, char* to);
  int (*open)         (struct kc_file_t* self, char* name, unsigned int mode);
  int (*read)         (struct kc_file_t* self, char** buffer);
  int (*write)        (struct kc_file_t* self, char* buffer);
};

struct kc_file_t* new_file      ();
void              destroy_file  (struct kc_file_t* file);

//---------------------------------------------------------------------------//

#endif /* KC_FILE_H */
