// This file is part of keepcoding_core
// ==================================
//
// logger.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/logger/logger.h"
#include "../../hdrs/common.h"

#include <time.h>

//--- MARK: PRIVATE MEMBER METHODS PROTOTYPES -------------------------------//

static int display_debug_message    (struct kc_logger_t* self, const int index, const int line, const char* func);
static int display_error_message    (struct kc_logger_t* self, const int index, const int line, const char* func);
static int display_fatal_message    (struct kc_logger_t* self, const int index, const int line, const char* func);
static int display_info_message     (struct kc_logger_t* self, const int index, const int line, const char* func);
static int display_warning_message  (struct kc_logger_t* self, const int index, const int line, const char* func);

//--- MARK: PUBLIC MEMBER METHODS PROTOTYPES --------------------------------//

void log_debug    (const char* title, const char* description, const char* file, const int line, const char* func);
void log_error    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_fatal    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_info     (const char* title, const char* description, const char* file, const int line, const char* func);
void log_warning  (const char* warning, const char* description, const char* file, const int line, const char* func);
int  log_to_file  (const char* filename, const char* log, const char* message, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

struct kc_logger_t* new_logger(const char** exceptions, const char** descriptions, const char* file)
{
  // create a ConsoleLog instance to be returned
  struct kc_logger_t* new_log = malloc(sizeof(struct kc_logger_t));

  // confirm that there is memory to allocate
  if (new_log == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // assign the exceptions and descriptions arrays
  new_log->_exceptions   = exceptions;
  new_log->_descriptions = descriptions;
  new_log->_file         = file;

  // assign the public member methods
  new_log->debug   = display_debug_message;
  new_log->error   = display_error_message;
  new_log->fatal   = display_fatal_message;
  new_log->info    = display_info_message;
  new_log->warning = display_warning_message;

  return new_log;
}

//---------------------------------------------------------------------------//

void destroy_logger(struct kc_logger_t* log)
{
  // free the memory only if the log is not null reference
  if (log == NULL)
  {
    log_fatal(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return;
  }

  free(log);
}

//---------------------------------------------------------------------------//

int display_debug_message(struct kc_logger_t* self, const int index,
   const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_debug(self->_exceptions[index],
      self->_descriptions[index], self->_file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_error_message(struct kc_logger_t* self, const int index,
   const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_error(self->_exceptions[index],
      self->_descriptions[index], self->_file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_fatal_message(struct kc_logger_t* self, const int index,
   const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_fatal(self->_exceptions[index],
      self->_descriptions[index], self->_file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_info_message(struct kc_logger_t* self, const int index,
   const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_info(self->_exceptions[index],
      self->_descriptions[index], self->_file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_warning_message(struct kc_logger_t* self, const int index,
   const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_warning(self->_exceptions[index],
      self->_descriptions[index], self->_file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void log_debug(const char* title, const char* description, const char* file,
   const int line, const char* func)
{
  printf("\n");
  printf("[DEBUG] %s:%d in function ‘%s’ \n", file, line, func);
  printf("[%s] %s\n", title, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_error(const char* exception, const char* description, const char* file,
   const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[ERROR] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m[%s] %s \033[0m \n", exception, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_fatal(const char* exception, const char* description, const char* file,
   const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[FATAL] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m[%s] %s \033[0m \n", exception, description);
  printf("\n");
  exit(1);
}

//---------------------------------------------------------------------------//

void log_info(const char* title, const char* description, const char* file,
   const int line, const char* func)
{
  printf("\n");
  printf("[INFO] %s:%d in function ‘%s’ \n", file, line, func);
  printf("[%s] %s\n", title, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_warning(const char* warning, const char* description, const char* file,
   const int line, const char* func)
{
  // use the \033[33m ANSI escape code for yellow color
  printf("\n");
  printf("\033[33m[WARNING] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[33m[%s] %s \033[0m \n", warning, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

int log_to_file(const char* filename, const char* log, const char* message,
    const char* file, const int line, const char* func)
{
  // open the file in append mode (creates a new file if it doesn't exist)
  FILE *write_file = fopen(filename, "a");

  // print an error and exit the program
  if (write_file == NULL)
  {
    log_error(err[KC_CANNOT_OPEN_FILE], log_err[KC_CANNOT_OPEN_FILE],
        __FILE__, __LINE__, __func__);

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
  fprintf(write_file, "\n");
  fprintf(write_file, "[%s]\n", time_buffer);
  fprintf(write_file, "%s: in function ‘%s’\n", file, func);
  fprintf(write_file, "%s:%d message: %s\n", file, line, log);
  fprintf(write_file, "  %s\n", message);

  // close the file
  fclose(write_file);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
