// This file is part of keepcoding_core
// ==================================
//
// logger.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * The ConsoleLog structure, provides a generic logging mechanism with various
 * functionalities. It allows users to log messages with additional contextual
 * information such as file name, line number, and function name. The structure
 * contains function pointers for different logging operations.
 *
 * It is designed to enhance debugging and error reporting by providing
 * context-specific information for each logged message.
 */

#ifndef KC_LOGGER_T_H
#define KC_LOGGER_T_H

#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------//

struct kc_logger_t
{
  const char** exceptions;
  const char** descriptions;
  const char* file;

  int (*debug)    (struct kc_logger_t* self, const int index, const int line, const char* func);
  int (*error)    (struct kc_logger_t* self, const int index, const int line, const char* func);
  int (*fatal)    (struct kc_logger_t* self, const int index, const int line, const char* func);
  int (*info)     (struct kc_logger_t* self, const int index, const int line, const char* func);
  int (*warning)  (struct kc_logger_t* self, const int index, const int line, const char* func);
};

struct kc_logger_t* new_logger      (const char** exceptions, const char** descriptions, const char* file);
void                destroy_logger  (struct kc_logger_t* log);

void log_debug    (const char* title, const char* description, const char* file, const int line, const char* func);
void log_error    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_fatal    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_info     (const char* title, const char* description, const char* file, const int line, const char* func);
void log_warning  (const char* warning, const char* description, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

/*
 * The log_to_file function offers a flexible and reusable logging solution,
 * allowing developers to customize log output to specific files based on their
 * requirements. It is designed to enhance debugging, error reporting, and
 * information gathering in C applications.
 */

int log_to_file  (const char* file_path, const char* log, const char* message, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

#endif /* KC_LOGGER_T_H */
