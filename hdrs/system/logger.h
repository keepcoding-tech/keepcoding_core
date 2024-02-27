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

#define KC_DEBUG_LOG    " [DEBUG] "
#define KC_ERROR_LOG    " [ERROR] "
#define KC_FATAL_LOG    " [FATAL] "
#define KC_INFO_LOG     " [INFO] "
#define KC_WARNING_LOG  " [WARNING] "

//---------------------------------------------------------------------------//

struct kc_logger_t
{
  const char*  _log_file;

  int (*log)  (struct kc_logger_t* self, const char* level, const char* description, const char* file, const int line, const char* func);
};

struct kc_logger_t* new_logger      (const char* log_file);
void                destroy_logger  (struct kc_logger_t* log);

void console_log_debug    (const char* description, const char* file, const int line, const char* func);
void console_log_error    (const char* description, const char* file, const int line, const char* func);
void console_log_fatal    (const char* description, const char* file, const int line, const char* func);
void console_log_info     (const char* description, const char* file, const int line, const char* func);
void console_log_warning  (const char* description, const char* file, const int line, const char* func);

#define log_debug(description)    console_log_debug   (description, __FILE__, __LINE__, __func__)
#define log_error(description)    console_log_error   (description, __FILE__, __LINE__, __func__)
#define log_fatal(description)    console_log_fatal   (description, __FILE__, __LINE__, __func__)
#define log_info(description)     console_log_info    (description, __FILE__, __LINE__, __func__)
#define log_warning(description)  console_log_warning (description, __FILE__, __LINE__, __func__)

//---------------------------------------------------------------------------//

/*
 * The log_to_file function offers a flexible and reusable logging solution,
 * allowing developers to customize log output to specific files based on their
 * requirements. It is designed to enhance debugging, error reporting, and
 * information gathering in C applications.
 */

int log_to_file  (const char* file_path, const char* level, const char* description, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

#endif /* KC_LOGGER_T_H */
