// This file is part of keepcoding_core
// ==================================
//
// file.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/common.h"
#include "../../hdrs/logger/logger.h"
#include "../../hdrs/system/file.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int close_file     (struct kc_file_t* self);
static int create_path    (struct kc_file_t* self, char* path);
static int delete_file    (struct kc_file_t* self);
static int delete_path    (struct kc_file_t* self, char* path);
static int get_file_mode  (struct kc_file_t* self, int* mode);
static int get_file_name  (struct kc_file_t* self, char** name);
static int get_file_path  (struct kc_file_t* self, char** path);
static int get_opened     (struct kc_file_t* self, bool* is_open);
static int open_file      (struct kc_file_t* self, char* name, unsigned int mode);
static int read_file      (struct kc_file_t* self, char** buffer);
static int write_file     (struct kc_file_t* self, char* buffer);

//---------------------------------------------------------------------------//

struct kc_file_t* new_file()
{
  // create a file instance to be returned
  struct kc_file_t* file = malloc(sizeof(struct kc_file_t));

  if (file == NULL)
  {
    log_error(err[KC_OUT_OF_MEMORY], log_err[KC_OUT_OF_MEMORY],
      __FILE__, __LINE__, __func__);

    return NULL;
  }

  struct kc_logger_t* log = new_logger(err, log_err, __FILE__);

  // assigns the public member fields
  file->_log   = log;
  file->file   = NULL;
  file->name   = NULL;
  file->path   = NULL;
  file->mode   = KC_FILE_NOT_FOUND;
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

void destroy_file(struct kc_file_t* file)
{
  if (file == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return;
  }

  destroy_logger(file->_log);

  // close the file if still open
  file->close(file);

  free(file->name);
  free(file);
}

//---------------------------------------------------------------------------//

int close_file(struct kc_file_t* self)
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
    self->mode   = KC_FILE_NOT_FOUND;
    self->opened = false;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int create_path(struct kc_file_t* self, char* path)
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
    return KC_INVALID;
  }

  self->path = (char*)malloc(sizeof(char) * (strlen(path) + 1));
  if (self->path == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // Save the path
  strcpy(self->path, path);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int delete_file(struct kc_file_t* self)
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
    self->_log->warning(self->_log, KC_FILE_NOT_FOUND, __LINE__, __func__);

    return KC_FILE_NOT_FOUND;
  }

  free(self->name);
  self->name = NULL;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int delete_path(struct kc_file_t* self, char* path)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;

  // open the directory
  if ((dir = opendir(path)) == NULL)
  {
    self->_log->error(self->_log, KC_CANT_OPEN_DIR, __LINE__, __func__);
    return KC_CANT_OPEN_DIR;
  }

  // iterate over the entries in the directory
  while ((entry = readdir(dir)) != NULL)
  {
    // skip "." and ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    // construct the full path of the entry
    char entry_path[KC_MAX_PATH];
    snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

    // get information about the entry
    if (stat(entry_path, &statbuf) == -1)
    {
      self->_log->error(self->_log, KC_FATAL_ERROR, __LINE__, __func__);
      return KC_FATAL_ERROR;
    }

    // recursively delete subdirectories
    if (S_ISDIR(statbuf.st_mode))
    {
      delete_path(self, entry_path);
    }
    else
    {
      // delete regular files
      if (unlink(entry_path) == -1)
      {
        self->_log->error(self->_log, KC_FATAL_ERROR, __LINE__, __func__);
        return KC_FATAL_ERROR;
      }
    }
  }

  // close the directory
  closedir(dir);

  // delete the current directory
  if (rmdir(path) == -1)
  {
    self->_log->error(self->_log, KC_FATAL_ERROR, __LINE__, __func__);
    return KC_FATAL_ERROR;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_mode(struct kc_file_t* self, int* mode)
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

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_name(struct kc_file_t* self, char** name)
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

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_file_path(struct kc_file_t* self, char** path)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // only if the file is open
  (*path) = self->path;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int get_opened(struct kc_file_t* self, bool* is_open)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  (*is_open) = self->opened;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int open_file(struct kc_file_t* self, char* name, unsigned int mode)
{
  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  const char* tmp_mode = NULL;

  // create new file, fail if exists
  if (mode & KC_FILE_CREATE_NEW)
  {
    tmp_mode   = "wx";
    self->mode = KC_FILE_CREATE_NEW;
  }

  // create new file, overwrite if exists
  if (mode & KC_FILE_CREATE_ALWAYS)
  {
    tmp_mode   = "w";
    self->mode = KC_FILE_CREATE_ALWAYS;
  }

  // open existing file, fail if not exists
  if (mode & KC_FILE_OPEN_EXISTING)
  {
    tmp_mode   = "r";
    self->mode = KC_FILE_OPEN_EXISTING;
  }

  // open existing file or create new
  if (mode & KC_FILE_OPEN_ALWAYS)
  {
    tmp_mode   = "a+";
    self->mode = KC_FILE_OPEN_ALWAYS;
  }

  // open existing file for read ony
  if (mode & KC_FILE_READ)
  {
    tmp_mode   = "r";
    self->mode = KC_FILE_READ;
  }

  // open existing file for write only
  if (mode & KC_FILE_WRITE)
  {
    tmp_mode   = "w";
    self->mode = KC_FILE_WRITE;
  }

  if (self->mode == KC_FILE_NOT_FOUND || tmp_mode == NULL)
  {
    // invalid mode provided
    self->_log->error(self->_log, KC_INVALID_ARGUMENT, __LINE__, __func__);

    return KC_INVALID_ARGUMENT;
  }

  // if a file was already opened, close it first
  if (self->opened == true)
  {
    fclose(self->file);
  }

  self->file = fopen(name, tmp_mode);

  if (self->file == NULL)
  {
    // file opening failed
    self->_log->error(self->_log, KC_FILE_NOT_FOUND, __LINE__, __func__);

    return KC_INVALID_ARGUMENT;
  }

  // save the file name
  self->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));
  if (self->name == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  strcpy(self->name, name);
  self->opened = true; // file is open

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int read_file(struct kc_file_t* self, char** buffer)
{
  int ret = KC_FILE_NOT_FOUND;

  if (self == NULL)
  {
    log_error(err[KC_NULL_REFERENCE], log_err[KC_NULL_REFERENCE],
      __FILE__, __LINE__, __func__);

    return KC_NULL_REFERENCE;
  }

  // open the file in "read" mode
  ret = open_file(self, self->name, KC_FILE_READ);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  fseek(self->file, 0, SEEK_END);
  long file_size = ftell(self->file);

  // Error determining file size
  if (file_size == -1)
  {
    return KC_OVERFLOW;
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

    return KC_OVERFLOW;
  }

  // Null-terminate the content
  (*buffer)[file_size] = '\0';

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int write_file(struct kc_file_t* self, char* buffer)
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
    return KC_IO_ERROR;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
