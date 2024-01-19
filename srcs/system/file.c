// This file is part of keepcoding_core
// ==================================
//
// file.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/error.h"
#include "../../hdrs/logger/console_log.h"
#include "../../hdrs/system/file.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int close_file     (struct File* self);
static int create_path    (struct File* self, char* path);
static int delete_file    (struct File* self);
static int delete_path    (struct File* self, char* path);
static int get_file_mode  (struct File* self, int* mode);
static int get_file_name  (struct File* self, char** name);
static int get_file_path  (struct File* self, char** path);
static int get_opened     (struct File* self, bool* is_open);
static int open_file      (struct File* self, char* name, unsigned int mode);
static int read_file      (struct File* self, char** buffer);
static int write_file     (struct File* self, char* buffer);

//---------------------------------------------------------------------------//

struct File* new_file()
{
  // create a file instance to be returned
  struct File* file = malloc(sizeof(struct File));

  if (file == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
      __FILE__, __LINE__, __func__);

    return NULL;
  }

  struct ConsoleLog* log = new_console_log(err, log_err, __FILE__);

  // assigns the public member fields
  file->log    = log;
  file->file   = NULL;
  file->name   = NULL;
  file->path   = NULL;
  file->mode   = KC_FILE_INVALID;
  file->opened = false;

  // assigns the public member methods
  file->close       = close_file;
  file->create_path = create_path;
  file->delete      = delete_file;
  file->delete_path = delete_path;
  file->get_mode    = get_file_mode;
  file->get_name    = get_file_name;
  file->get_path    = get_file_path;
  file->is_open     = get_opened;
  file->move        = NULL;
  file->open        = open_file;
  file->read        = read_file;
  file->write       = write_file;

  return file;
}

//---------------------------------------------------------------------------//

void destroy_file(struct File* file)
{
  if (file == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return;
  }

  destroy_console_log(file->log);

  // close the file if still open
  file->close(file);

  free(file->name);
  free(file);
}

//---------------------------------------------------------------------------//

int close_file(struct File* self)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  if (self->file != NULL && self->opened == true)
  {
    fclose(self->file);

    self->file   = NULL;
    self->mode   = KC_FILE_INVALID;
    self->opened = false;
  }

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int create_path(struct File* self, char* path)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // free the path before creating a new one
  if (self->path != NULL)
  {
    free(self->path);
  }

  if (mkdir(path, 0777) != 0)
  {
    return KC_FILE_INVALID;
  }

  self->path = (char*)malloc(sizeof(char) * (strlen(path) + 1));
  if (self->path == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // Save the path
  strcpy(self->path, path);

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int delete_file(struct File* self)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // close the file before deleting it
  close_file(self);

  if (remove(self->name) != 0)
  {
    self->log->warning(self->log, KC_FILE_NOT_FOUND, __LINE__, __func__);

    return KC_FILE_INVALID;
  }

  free(self->name);
  self->name = NULL;

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int delete_path(struct File* self, char* path)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_mode(struct File* self, int* mode)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // the file must be open
  if (self->opened == false)
  {
    return KC_FILE_CLOSED;
  }

  // only if the file is open
  (*mode) = self->mode;

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_name(struct File* self, char** name)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // the file must be open
  if (self->opened == false)
  {
    return KC_FILE_CLOSED;
  }

  // only if the file is open
  (*name) = self->name;

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_path(struct File* self, char** path)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // only if the file is open
  (*path) = self->path;

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_opened(struct File* self, bool* is_open)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  (*is_open) = self->opened;

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int open_file(struct File* self, char* name, unsigned int mode)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  const char* tmp_mode = NULL;

  // Create new file, fail if exists
  if (mode & KC_FILE_CREATE_NEW)
  {
    tmp_mode   = "wx";
    self->mode = KC_FILE_CREATE_NEW;
  }

  // Create new file, overwrite if exists
  if (mode & KC_FILE_CREATE_ALWAYS)
  {
    tmp_mode   = "w";
    self->mode = KC_FILE_CREATE_ALWAYS;
  }

  // Open existing file, fail if not exists
  if (mode & KC_FILE_OPEN_EXISTING)
  {
    tmp_mode   = "r";
    self->mode = KC_FILE_OPEN_EXISTING;
  }

  // Open existing file or create new
  if (mode & KC_FILE_OPEN_ALWAYS)
  {
    tmp_mode   = "a+";
    self->mode = KC_FILE_OPEN_ALWAYS;
  }

  // Open existing file for read ony
  if (mode & KC_FILE_READ)
  {
    tmp_mode   = "r";
    self->mode = KC_FILE_READ;
  }

  // Open existing file for write only
  if (mode & KC_FILE_WRITE)
  {
    tmp_mode   = "w";
    self->mode = KC_FILE_WRITE;
  }

  if (self->mode == KC_FILE_INVALID || tmp_mode == NULL)
  {
    // Invalid mode provided
    self->log->error(self->log, KC_INVALID_ARGUMENT, __LINE__, __func__);

    return KC_FILE_INVALID;
  }

  // if a file was already opened, close it first
  if (self->opened == true)
  {
    fclose(self->file);
  }

  self->file = fopen(name, tmp_mode);

  if (self->file == NULL)
  {
    // File opening failed
    self->log->error(self->log, KC_FILE_NOT_FOUND, __LINE__, __func__);

    return KC_FILE_INVALID;
  }

  // Save the file name
  self->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));
  if (self->name == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  strcpy(self->name, name);
  self->opened = true; // file is open

  return KC_FILE_SUCCESS; // Return success status
}

//---------------------------------------------------------------------------//

int read_file(struct File* self, char** buffer)
{
  int ret = KC_FILE_INVALID;

  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // open the file in "read" mode
  ret = open_file(self, self->name, KC_FILE_READ);
  if (ret == KC_FILE_INVALID)
  {
    return ret;
  }

  fseek(self->file, 0, SEEK_END);
  long file_size = ftell(self->file);

  // Error determining file size
  if (file_size == -1)
  {
    return KC_BUFFER_OVERFLOW;
  }

  // Reset file pointer to the beginning
  fseek(self->file, 0, SEEK_SET);

  (*buffer) = (char*)malloc(file_size + 1);

  // Memory allocation failed
  if (*buffer == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // Read file content into buffer
  size_t bytes_read = fread((void*)*buffer, 1, file_size, self->file);

  // Error reading file content
  if (bytes_read != (size_t)file_size)
  {
    free(buffer);

    return KC_BUFFER_OVERFLOW;
  }

  // Null-terminate the content
  (*buffer)[file_size] = '\0';

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//

int write_file(struct File* self, char* buffer)
{
  if (self == NULL || buffer == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  size_t bytes_written = fwrite(buffer, 1, strlen(buffer), self->file);

  // Error writing content to file
  if (bytes_written != strlen(buffer))
  {
    return KC_FILE_INVALID;
  }

  return KC_FILE_SUCCESS;
}

//---------------------------------------------------------------------------//
