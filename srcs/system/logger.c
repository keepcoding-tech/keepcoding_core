// This file is part of keepcoding_core
// ==================================
//
// logger.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/system/logger.h"
#include "../../hdrs/common.h"

#include <time.h>
#include <string.h>

//--- MARK: PRIVATE MEMBER METHODS PROTOTYPES -------------------------------//

void console_log_debug    (const char* description, const char* file, const int line, const char* func);
void console_log_error    (const char* description, const char* file, const int line, const char* func);
void console_log_fatal    (const char* description, const char* file, const int line, const char* func);
void console_log_info     (const char* description, const char* file, const int line, const char* func);
void console_log_warning  (const char* description, const char* file, const int line, const char* func);

static int save_log  (struct kc_logger_t* self, const char* level, const int error_code, const char* file, const int line, const char* func);

int  log_to_file  (const char* filename, const char* level, const char* message, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

struct kc_logger_t* new_logger(const char* log_file)
{
  // create a ConsoleLog instance to be returned
  struct kc_logger_t* new_log = malloc(sizeof(struct kc_logger_t));

  // confirm that there is memory to allocate
  if (new_log == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  // confirm that the log file is not empty
  if (strlen(log_file) <= 0)
  {
    log_error(KC_UNDERFLOW_LOG);
    return NULL;
  }

  // assign the exceptions and descriptions arrays
  new_log->_log_file = log_file;

  // assign the public member methods
  new_log->log = save_log;

  return new_log;
}

//---------------------------------------------------------------------------//

void destroy_logger(struct kc_logger_t* log)
{
  // free the memory only if the log is not null reference
  if (log == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(log);
}

//---------------------------------------------------------------------------//

int save_log(struct kc_logger_t* self, const char* level, const int error_code, const char* file, const int line, const char* func)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  if (error_code < -1 || error_code >= get_kc_error_count())
  {
    printf("%d", error_code);
    log_error(KC_INVALID_ARGUMENT_LOG);
    return KC_INVALID_ARGUMENT;
  }

  if (line < 0)
  {
    log_error(KC_INVALID_ARGUMENT_LOG);
    return KC_INVALID_ARGUMENT;
  }

  return log_to_file(self->_log_file, level,
    kc_error_msg[error_code + 1], file, line, func);
}

//---------------------------------------------------------------------------//

void console_log_debug(const char* description, const char* file, const int line, const char* func)
{
  printf("\n");
  printf("[DEBUG] %s:%d in function ‘%s’ \n", file, line, func);
  printf("  -> %s\n", description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void console_log_error(const char* description, const char* file, const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[ERROR] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m  -> %s \033[0m \n", description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void console_log_fatal(const char* description, const char* file, const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[FATAL] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m  -> %s \033[0m \n", description);
  printf("\n");
  exit(1);
}

//---------------------------------------------------------------------------//

void console_log_info(const char* description, const char* file, const int line, const char* func)
{
  printf("\n");
  printf("[INFO] %s:%d in function ‘%s’ \n", file, line, func);
  printf("  -> %s\n", description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void console_log_warning(const char* description, const char* file, const int line, const char* func)
{
  // use the \033[33m ANSI escape code for yellow color
  printf("\n");
  printf("\033[33m[WARNING] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[33m  -> %s \033[0m \n", description);
  printf("\n");
}

//---------------------------------------------------------------------------//

int log_to_file(const char* filename, const char* level, const char* description, const char* file, const int line, const char* func)
{
  // open the file in append mode (creates a new file if it doesn't exist)
  FILE *write_file = fopen(filename, "a");

  // print an error and exit the program
  if (write_file == NULL)
  {
    log_error(KC_CANNOT_OPEN_FILE_LOG);
    return KC_CANNOT_OPEN_FILE;
  }

  // get the current timestamp
  time_t now;
  struct tm *tm_info;
  char time_buffer[30];
  time(&now);

  // convert the timestamp to a human-readable string
  tm_info = localtime(&now);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  // writing data the log to the file
  fprintf(write_file, "\n[%s] %s: in function ‘%s’\n", time_buffer, level, func);
  fprintf(write_file, "%s:%d -> %s\n", file, line, description);

  // close the file
  fclose(write_file);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
