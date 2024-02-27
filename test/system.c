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
      ok(file->_log != NULL);
      ok(file->file == NULL);
      ok(file->name == NULL);
      ok(file->mode == KC_FILE_NOT_FOUND);
      ok(file->opened == false);

      destroy_file(file);
    }

    subtest("test close()")
    {
      struct kc_file_t* file = new_file();
      int rez = KC_INVALID;

      rez = file->open(file, "test_close", KC_FILE_CREATE_NEW);

      ok(rez == KC_SUCCESS);
      ok(file->file != NULL);
      ok(file->mode & KC_FILE_CREATE_NEW);

      rez = file->close(file);

      ok(rez == KC_SUCCESS);
      ok(file->opened == false);
      ok(file->file == NULL);

      rez = file->delete(file);
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test create_path()")
    {
      struct kc_file_t* file = new_file();
      int rez = KC_INVALID;

      rez = file->create_path(file, "test_files");
      ok(rez == KC_SUCCESS);

      rez = file->delete_path(file, "/workspaces/keepcoding_core/test_files");
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test delete()")
    {
      struct kc_file_t* file = new_file();
      int rez = KC_INVALID;

      rez = file->open(file, "test_delete", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->delete(file);

      ok(rez == KC_SUCCESS);
      ok(file->file == NULL);

      destroy_file(file);
    }

    subtest("test get_mode()")
    {
      struct kc_file_t* file = new_file();

      int rez  = KC_INVALID;
      int mode = KC_INVALID;

      rez = file->open(file, "test_get_mode", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->get_mode(file, &mode);

      ok(rez == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_NEW);

      rez = file->open(file, "test_get_mode", KC_FILE_CREATE_ALWAYS);
      ok(rez == KC_SUCCESS);

      rez = file->get_mode(file, &mode);

      ok(rez == KC_SUCCESS);
      ok(mode == KC_FILE_CREATE_ALWAYS);

      rez = file->open(file, "test_get_mode", KC_FILE_OPEN_EXISTING);
      ok(rez == KC_SUCCESS);

      rez = file->get_mode(file, &mode);

      ok(rez == KC_SUCCESS);
      ok(mode & KC_FILE_OPEN_EXISTING);

      rez = file->open(file, "test_get_mode", KC_FILE_OPEN_ALWAYS);
      ok(rez == KC_SUCCESS);

      rez = file->get_mode(file, &mode);

      ok(rez == KC_SUCCESS);
      ok(mode == KC_FILE_OPEN_ALWAYS);

      rez = file->delete(file);
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test get_name()")
    {
      struct kc_file_t* file = new_file();

      int   rez = KC_INVALID;
      char* name;

      rez = file->open(file, "test_get_name", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->get_name(file, &name);

      ok(rez == KC_SUCCESS);
      ok(strcmp(name, "test_get_name") == 0);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test get_path()")
    {
      struct kc_file_t* file = new_file();

      int   rez = KC_INVALID;
      char* path;

      rez = file->create_path(file, "sys_test");
      ok(rez == KC_SUCCESS);

      rez = file->create_path(file, "sys_test/get");
      ok(rez == KC_SUCCESS);

      rez = file->create_path(file, "sys_test/get/path");
      ok(rez == KC_SUCCESS);

      rez = file->get_path(file, &path);

      ok(rez == KC_SUCCESS);
      ok(strcmp(path, "sys_test/get/path") == 0);

      rez = file->delete_path(file, "/workspaces/keepcoding_core/sys_test");
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test is_open()")
    {
      struct kc_file_t* file = new_file();

      int  rez     = KC_INVALID;
      bool is_open = false;

      rez = file->is_open(file, &is_open);

      ok(rez == KC_SUCCESS);
      ok(is_open == false);

      rez = file->open(file, "test_close", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->is_open(file, &is_open);

      ok(rez == KC_SUCCESS);
      ok(is_open == true);

      file->close(file);

      rez = file->is_open(file, &is_open);

      ok(rez == KC_SUCCESS);
      ok(is_open == false);

      rez = file->delete(file);
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test open()")
    {
      struct kc_file_t* file = new_file();
      int rez = KC_INVALID;

      note("Create New")
      rez = file->open(file, "test_open", KC_FILE_CREATE_NEW);

      ok(rez == KC_SUCCESS);
      ok(file->mode & KC_FILE_CREATE_NEW);

      note("Create Always")
      rez = file->open(file, "test_open", KC_FILE_CREATE_ALWAYS);

      ok(rez == KC_SUCCESS);
      ok(file->mode & KC_FILE_CREATE_ALWAYS);

      note("Open Existing")
      rez = file->open(file, "test_open", KC_FILE_OPEN_EXISTING);

      ok(rez == KC_SUCCESS);
      ok(file->mode & KC_FILE_OPEN_EXISTING);

      note("Open Always")
      rez = file->open(file, "test_open", KC_FILE_OPEN_ALWAYS);

      ok(rez == KC_SUCCESS);
      ok(file->mode & KC_FILE_OPEN_ALWAYS);

      rez = file->delete(file);
      ok(rez == KC_SUCCESS);

      destroy_file(file);
    }

    subtest("test read()")
    {
      struct kc_file_t* file = new_file();

      int   rez = KC_INVALID;
      char* buffer;

      rez = file->open(file, "test_read", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->write(file, "This is just a read test");
      ok(rez == KC_SUCCESS);

      rez = file->read(file, &buffer);

      ok(rez == KC_SUCCESS);
      ok(strcmp(buffer, "This is just a read test") == 0);

      file->delete(file);
      destroy_file(file);
    }

    subtest("test write()")
    {
      struct kc_file_t* file = new_file();

      int   rez = KC_INVALID;
      char* buffer;

      rez = file->open(file, "test_write", KC_FILE_CREATE_NEW);
      ok(rez == KC_SUCCESS);

      rez = file->write(file, "This is just a write test");
      ok(rez == KC_SUCCESS);

      rez = file->read(file, &buffer);
      ok(rez == KC_SUCCESS);

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
      logger->log(logger, " [TEST] ", "This is a test description! XD",
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
      test = __FILE__ ":350 -> This is a test description! XD";
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

      int rez = KC_INVALID;

      rez = thread->start(thread, &test_thread_func, NULL);

      ok(rez == KC_SUCCESS);

      // terminate the thread
      rez = pthread_join(thread->_thread, NULL);
      ok(rez == KC_SUCCESS);

      destroy_thread(thread);
    }

    subtest("test stop()")
    {
      struct kc_thread_t* thread = new_thread();

      int rez = KC_INVALID;

      // create a thread
      rez = pthread_create(&thread->_thread, NULL, &test_thread_func, NULL);
      ok(rez == KC_SUCCESS);

      rez = thread->stop(thread);
      ok(rez == KC_SUCCESS);

      destroy_thread(thread);
    }

    subtest("test join()")
    {
      struct kc_thread_t* thread = new_thread();

      int rez = KC_INVALID;
      int val = 1;
      void* arg = &val;

      // create a thread
      rez = thread->start(thread, &test_thread_func, arg);
      ok(rez == KC_SUCCESS);

      rez = thread->join(thread, &arg);
      ok(rez == KC_SUCCESS);
      ok(*(int*)(arg) == 2); // the value was changed

      destroy_thread(thread);
    }

    subtest("test mutex")
    {
      struct kc_thread_t* thread1 = new_thread();
      struct kc_thread_t* thread2 = new_thread();

      int rez = KC_INVALID;

      rez = thread1->start(thread1, &test_race_condition, NULL);
      ok(rez == KC_SUCCESS);
      rez = thread2->start(thread2, &test_race_condition, NULL);
      ok(rez == KC_SUCCESS);

      rez = thread1->join(thread1, NULL);
      ok(rez == KC_SUCCESS);
      rez = thread2->join(thread2, NULL);
      ok(rez == KC_SUCCESS);

      // using mutex, this should be true
      ok(race_counter == 2000000);

      destroy_thread(thread1);
      destroy_thread(thread2);
    }

    done_testing();
  }

  return 0;
}
