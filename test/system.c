// This file is part of keepcoding_core
// ==================================
//
// system.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/system/file.h"

#include "../../hdrs/common.h"
#include "../../hdrs/test.h"

#include <stdio.h>
#include <string.h>

int main()
{
  testgroup("kc_file_t")
  {
    subtest("test init/desc")
    {
      struct kc_file_t* file = new_file();

      ok(file != NULL);
      ok(file->log != NULL);
      ok(file->file == NULL);
      ok(file->name == NULL);
      ok(file->mode == KC_INVALID);
      ok(file->opened == false);

      destroy_file(file);
    }

    subtest("test create_path()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      ret = file->create_path(file, "test_files");

      ok(ret == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test close()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      file->open(file, "test_close", KC_FILE_CREATE_NEW);

      ok(file->file != NULL);
      ok(file->mode & KC_FILE_CREATE_NEW);

      ret = file->close(file);

      ok(ret == KC_SUCCESS);
      ok(file->opened == false);
      ok(file->file == NULL);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test delete()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      file->open(file, "test_delete", KC_FILE_CREATE_NEW);

      ret = file->delete(file);

      ok(ret == KC_SUCCESS);
      ok(file->file == NULL);

      destroy_file(file);
    }

    subtest("test get_mode()")
    {
      struct kc_file_t* file = new_file();

      int ret  = KC_INVALID;
      int mode = KC_INVALID;

      file->open(file, "test_get_mode", KC_FILE_CREATE_NEW);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_NEW);

      file->open(file, "test_get_mode", KC_FILE_CREATE_ALWAYS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_ALWAYS);

      file->open(file, "test_get_mode", KC_FILE_OPEN_EXISTING);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode & KC_FILE_OPEN_EXISTING);

      file->open(file, "test_get_mode", KC_FILE_OPEN_ALWAYS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_OPEN_ALWAYS);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test get_name()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* name;

      file->open(file, "test_get_name", KC_FILE_CREATE_NEW);
      ret = file->get_name(file, &name);

      ok(ret == KC_SUCCESS);
      ok(strcmp(name, "test_get_name") == 0);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test get_path()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* path;

      file->create_path(file, "test");
      file->create_path(file, "test/get");
      file->create_path(file, "test/get/path");

      ret = file->get_path(file, &path);

      ok(ret == KC_SUCCESS);
      ok(strcmp(path, "test/get/path") == 0);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test is_open()")
    {
      struct kc_file_t* file = new_file();

      int  ret     = KC_INVALID;
      bool is_open = false;

      ret = file->is_open(file, &is_open);

      ok(ret == KC_SUCCESS);
      ok(is_open == false);

      file->open(file, "test_close", KC_FILE_CREATE_NEW);
      ret = file->is_open(file, &is_open);

      ok(ret == KC_SUCCESS);
      ok(is_open == true);

      file->close(file);

      ret = file->is_open(file, &is_open);

      ok(ret == KC_SUCCESS);
      ok(is_open == false);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test open()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      note("Create New")
      ret = file->open(file, "test_open", KC_FILE_CREATE_NEW);

      ok(ret == KC_SUCCESS);
      ok(file->mode & KC_FILE_CREATE_NEW);

      note("Create Always")
      ret = file->open(file, "test_open", KC_FILE_CREATE_ALWAYS);

      ok(ret == KC_SUCCESS);
      ok(file->mode & KC_FILE_CREATE_ALWAYS);

      note("Open Existing")
      ret = file->open(file, "test_open", KC_FILE_OPEN_EXISTING);

      ok(ret == KC_SUCCESS);
      ok(file->mode & KC_FILE_OPEN_EXISTING);

      note("Open Always")
      ret = file->open(file, "test_open", KC_FILE_OPEN_ALWAYS);

      ok(ret == KC_SUCCESS);
      ok(file->mode & KC_FILE_OPEN_ALWAYS);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test read()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* buffer;

      file->open(file, "test_read", KC_FILE_CREATE_NEW);
      file->write(file, "This is just a read test");

      ret = file->read(file, &buffer);

      ok(ret == KC_SUCCESS);
      ok(strcmp(buffer, "This is just a read test") == 0);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test write()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* buffer;

      file->open(file, "test_write", KC_FILE_CREATE_NEW);
      ret = file->write(file, "This is just a write test");

      ok(ret == KC_SUCCESS);

      file->read(file, &buffer);

      ok(strcmp(buffer, "This is just a write test") == 0);

      file->delete(file);
      destroy_file(file);
    }

    done_testing();
  }

  return 0;
}
