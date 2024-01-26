// This file is part of keepcoding_core
// ==================================
//
// console_log.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/logger/console_log.h"
#include "../../hdrs/common.h"

//--- MARK: PRIVATE MEMBER METHODS PROTOTYPES -------------------------------//

static int display_debug_message    (struct kc_console_log_t* self, const int index, const int line, const char* func);
static int display_error_message    (struct kc_console_log_t* self, const int index, const int line, const char* func);
static int display_fatal_message    (struct kc_console_log_t* self, const int index, const int line, const char* func);
static int display_info_message     (struct kc_console_log_t* self, const int index, const int line, const char* func);
static int display_warning_message  (struct kc_console_log_t* self, const int index, const int line, const char* func);

//--- MARK: PUBLIC MEMBER METHODS PROTOTYPES --------------------------------//

void log_debug    (const char* title, const char* description, const char* file, const int line, const char* func);
void log_error    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_fatal    (const char* exception, const char* description, const char* file, const int line, const char* func);
void log_info     (const char* title, const char* description, const char* file, const int line, const char* func);
void log_warning  (const char* warning, const char* description, const char* file, const int line, const char* func);

//---------------------------------------------------------------------------//

struct kc_console_log_t* new_console_log(const char** exceptions, const char** descriptions, const char* file)
{
  // create a ConsoleLog instance to be returned
  struct kc_console_log_t* new_log = malloc(sizeof(struct kc_console_log_t));

  // confirm that there is memory to allocate
  if (new_log == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
        __FILE__, __LINE__, __func__);

    return NULL;
  }

  // assign the exceptions and descriptions arrays
  new_log->exceptions   = exceptions;
  new_log->descriptions = descriptions;
  new_log->file         = file;

  // assign the public member methods
  new_log->debug   = display_debug_message;
  new_log->error   = display_error_message;
  new_log->fatal   = display_fatal_message;
  new_log->info    = display_info_message;
  new_log->warning = display_warning_message;

  return new_log;
}

//---------------------------------------------------------------------------//

void destroy_console_log(struct kc_console_log_t* log)
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

int display_debug_message(struct kc_console_log_t* self, const int index, const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_debug(self->exceptions[index],
      self->descriptions[index], self->file, line, func);
}

//---------------------------------------------------------------------------//

int display_error_message(struct kc_console_log_t* self, const int index, const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_error(self->exceptions[index],
      self->descriptions[index], self->file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_fatal_message(struct kc_console_log_t* self, const int index, const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_fatal(self->exceptions[index],
      self->descriptions[index], self->file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_info_message(struct kc_console_log_t* self, const int index, const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_info(self->exceptions[index],
      self->descriptions[index], self->file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int display_warning_message(struct kc_console_log_t* self, const int index, const int line, const char* func)
{
  // if the list reference is NULL, abort the application
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
        __FILE__, __LINE__, __func__);

    return KC_INVALID;
  }

  log_warning(self->exceptions[index],
      self->descriptions[index], self->file, line, func);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void log_debug(const char* title, const char* description, const char* file, const int line, const char* func)
{
  printf("\n");
  printf("[DEBUG] %s:%d in function ‘%s’ \n", file, line, func);
  printf("[%s] %s\n", title, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_error(const char* exception, const char* description, const char* file, const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[ERROR] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m[%s] %s \033[0m \n", exception, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_fatal(const char* exception, const char* description, const char* file, const int line, const char* func)
{
  // use the \033[31m ANSI escape code for red color
  printf("\n");
  printf("\033[31m[FATAL] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[31m[%s] %s \033[0m \n", exception, description);
  printf("\n");
  exit(1);
}

//---------------------------------------------------------------------------//

void log_info(const char* title, const char* description, const char* file, const int line, const char* func)
{
  printf("\n");
  printf("[INFO] %s:%d in function ‘%s’ \n", file, line, func);
  printf("[%s] %s\n", title, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

void log_warning(const char* warning, const char* description, const char* file, const int line, const char* func)
{
  // use the \033[33m ANSI escape code for yellow color
  printf("\n");
  printf("\033[33m[WARNING] %s:%d in function ‘%s’ \033[0m \n", file, line, func);
  printf("\033[33m[%s] %s \033[0m \n", warning, description);
  printf("\n");
}

//---------------------------------------------------------------------------//

