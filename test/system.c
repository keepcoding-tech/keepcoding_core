// This file is part of keepcoding_core
// ==================================
//
// system.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/system/file.h"
#include "../hdrs/system/logger.h"
#include "../hdrs/system/thread.h"

#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>

#define DEBUG "This is just a test description for debug! XD"
#define ERROR "This is just a test description for error! XD"
#define INFO  "This is just a test description for info! XD"
#define WARN  "This is just a test description for warning! XD"
#define FATAL "This is just a test description for fatal! XD\n"       \
  "After this test, the program will exit with a status code of 1.\n" \
  "This is GOOD!!"

void* test_thread_func(void* arg)
{
  note("starting thread...");

  if (arg != NULL)
  {
    int* res = malloc(sizeof(int));
    (*res) = *(int*)arg;
    (*res)++;

    return (void*)res;
  }

  return NULL;
}

int race_counter = 0;
pthread_mutex_t mutex;

void* test_race_condition()
{
  for (int i = 0; i < 1000000; ++i)
  {
    kc_mutex_lock
    ++race_counter;
    kc_mutex_unlock
  }
  return NULL;
}

int main()
{
  testgroup("kc_file_t")
  {
    subtest("test init/desc")
    {
      struct kc_file_t* file = new_file();

      ok(file != NULL);
      ok(file->_logger != NULL);
      ok(file->file == NULL);
      ok(file->name == NULL);
      ok(file->mode == KC_FILE_NOT_FOUND);
      ok(file->opened == false);

      destroy_file(file);
    }

    subtest("test close()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      ret = file->open(file, "test_close", KC_FILE_CREATE_NEW);

      ok(ret == KC_SUCCESS);
      ok(file->file != NULL);
      ok(file->mode & KC_FILE_CREATE_NEW);

      ret = file->close(file);

      ok(ret == KC_SUCCESS);
      ok(file->opened == false);
      ok(file->file == NULL);

      ret = file->delete(file);
      ok(ret == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test create_path()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      ret = file->create_path(file, "test_files");
      ok(ret == KC_SUCCESS);

      ret = file->delete_path(file, "/workspaces/keepcoding_core/test_files");
      ok(ret == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test delete()")
    {
      struct kc_file_t* file = new_file();
      int ret = KC_INVALID;

      ret = file->open(file, "test_delete", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

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

      ret = file->open(file, "test_get_mode", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_NEW);

      ret = file->open(file, "test_get_mode", KC_FILE_CREATE_ALWAYS);
      ok(ret == KC_SUCCESS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_ALWAYS);

      ret = file->open(file, "test_get_mode", KC_FILE_OPEN_EXISTING);
      ok(ret == KC_SUCCESS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode & KC_FILE_OPEN_EXISTING);

      ret = file->open(file, "test_get_mode", KC_FILE_OPEN_ALWAYS);
      ok(ret == KC_SUCCESS);

      ret = file->get_mode(file, &mode);

      ok(ret == KC_SUCCESS);
      ok(mode == KC_FILE_OPEN_ALWAYS);

      ret = file->delete(file);
      ok(ret == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test get_name()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* name;

      ret = file->open(file, "test_get_name", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

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

      ret = file->create_path(file, "sys_test");
      ok(ret == KC_SUCCESS);

      ret = file->create_path(file, "sys_test/get");
      ok(ret == KC_SUCCESS);

      ret = file->create_path(file, "sys_test/get/path");
      ok(ret == KC_SUCCESS);

      ret = file->get_path(file, &path);

      ok(ret == KC_SUCCESS);
      ok(strcmp(path, "sys_test/get/path") == 0);

      ret = file->delete_path(file, "/workspaces/keepcoding_core/sys_test");
      ok(ret == KC_SUCCESS);

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

      ret = file->open(file, "test_close", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

      ret = file->is_open(file, &is_open);

      ok(ret == KC_SUCCESS);
      ok(is_open == true);

      file->close(file);

      ret = file->is_open(file, &is_open);

      ok(ret == KC_SUCCESS);
      ok(is_open == false);

      ret = file->delete(file);
      ok(ret == KC_SUCCESS);

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

      ret = file->delete(file);
      ok(ret == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test read()")
    {
      struct kc_file_t* file = new_file();

      int   ret = KC_INVALID;
      char* buffer;

      ret = file->open(file, "test_read", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

      ret = file->write(file, "This is just a read test");
      ok(ret == KC_SUCCESS);

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

      ret = file->open(file, "test_write", KC_FILE_CREATE_NEW);
      ok(ret == KC_SUCCESS);

      ret = file->write(file, "This is just a write test");
      ok(ret == KC_SUCCESS);

      ret = file->read(file, &buffer);
      ok(ret == KC_SUCCESS);

      ok(strcmp(buffer, "This is just a write test") == 0);

      file->delete(file);
      destroy_file(file);
    }

    done_testing();
  }

  testgroup("kc_logger_t")
  {
    subtest("test init/desc")
    {
      struct kc_logger_t* log = new_logger("build/bin/test/logger.log");
      ok(strcmp(log->_log_file, "build/bin/test/logger.log") == 0);
      destroy_logger(log);
    }

    subtest("test log_to_file()")
    {
      const char* filename = "build/bin/test/logger.log";
      struct kc_logger_t* logger = new_logger(filename);

      // write the log to the file
      logger->log(logger, " [TEST] ", 0,
        __FILE__, __LINE__, __func__); // DO NOT MOVE THIS LINE!!!

      // check if the log was printed correctely
      FILE* read_file = fopen(filename, "r");
      char read_line[100];

      // skip the next two lines
      fgets(read_line, sizeof(read_line), read_file);
      fgets(read_line, sizeof(read_line), read_file);

      char* test = " [TEST] : in function ‘main’";
      for (int i = 0; i < strlen(test) - 1; ++i)
      {
        ok(read_line[i + 22] == test[i]);
      }

      fgets(read_line, sizeof(read_line), read_file);
      test = __FILE__ ":350 -> Successful completion of the process.";
      for (int i = 0; i < strlen(test) - 1; ++i)
      {
        ok(read_line[i] == test[i]);
      }

      destroy_logger(logger);
    }

    subtest("test debug()")
    {
      log_debug(DEBUG);
    }

    subtest("test error()")
    {
      log_error(ERROR);
    }

    subtest("test info()")
    {
      log_info(INFO);
    }

    subtest("test warning()")
    {
      log_warning(WARN);
    }

    subtest("test fatal()")
    {
      // this test will also exit with error code 1
      // uncomment only for locale testing
      //log_fatal(FATAL);
    }

    done_testing();
  }

  testgroup("kc_thread_t")
  {
    subtest("test init/desc")
    {
      struct kc_thread_t* thread = new_thread();
      destroy_thread(thread);
    }

    subtest("test start()")
    {
      struct kc_thread_t* thread = new_thread();

      int ret = KC_INVALID;

      ret = thread->start(thread, &test_thread_func, NULL);

      ok(ret == KC_SUCCESS);

      // terminate the thread
      ret = pthread_join(thread->_thread, NULL);
      ok(ret == KC_SUCCESS);

      destroy_thread(thread);
    }

    subtest("test stop()")
    {
      struct kc_thread_t* thread = new_thread();

      int ret = KC_INVALID;

      // create a thread
      ret = pthread_create(&thread->_thread, NULL, &test_thread_func, NULL);
      ok(ret == KC_SUCCESS);

      ret = thread->stop(thread);
      ok(ret == KC_SUCCESS);

      destroy_thread(thread);
    }

    subtest("test join()")
    {
      struct kc_thread_t* thread = new_thread();

      int ret = KC_INVALID;
      int val = 1;
      void* arg = &val;

      // create a thread
      ret = thread->start(thread, &test_thread_func, arg);
      ok(ret == KC_SUCCESS);

      ret = thread->join(thread, &arg);
      ok(ret == KC_SUCCESS);
      ok(*(int*)(arg) == 2); // the value was changed

      destroy_thread(thread);
    }

    subtest("test mutex")
    {
      struct kc_thread_t* thread1 = new_thread();
      struct kc_thread_t* thread2 = new_thread();

      int ret = KC_INVALID;

      ret = thread1->start(thread1, &test_race_condition, NULL);
      ok(ret == KC_SUCCESS);
      ret = thread2->start(thread2, &test_race_condition, NULL);
      ok(ret == KC_SUCCESS);

      ret = thread1->join(thread1, NULL);
      ok(ret == KC_SUCCESS);
      ret = thread2->join(thread2, NULL);
      ok(ret == KC_SUCCESS);

      // using mutex, this should be true
      ok(race_counter == 2000000);

      destroy_thread(thread1);
      destroy_thread(thread2);
    }

    done_testing();
  }

  return 0;
}
