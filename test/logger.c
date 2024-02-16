// This file is part of keepcoding_core
// ==================================
//
// logger.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/logger/logger.h"

#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <assert.h>
#include <string.h>

enum {
  TEST_DEBUG,
  TEST_ERROR,
  TEST_INFO,
  TEST_WARNING,
  TEST_FATAL
};

const char* ex[] = {
  "TEST_DEBUG",
  "TEST_ERROR",
  "TEST_INFO",
  "TEST_WARNING",
  "TEST_FATAL"
};

const char* log_ex[] = {
  "This is just a test description for debug! XD",
  "This is just a test description for error! XD",
  "This is just a test description for info! XD",
  "This is just a test description for warning! XD",
  "This is just a test description for fatal! XD\n"
    "After this test, the program will exit with a status code of 1.\n"
    "This is GOOD!!",
};

int main() {
  testgroup("kc_logger_t")
  {
    subtest("test init/desc")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);
      destroy_logger(log);
    }

    subtest("test log_to_file()")
    {
      const char* filename = "build/bin/test/test.log";

      // write the log to the file
      log_to_file(filename, "THIS IS JUST A TEST!!!",
        "This is a test description! XD",
        __FILE__, __LINE__, __func__); // DO NOT MOVE THIS LINE!!!

      // check if the log was printed correctely
      FILE* read_file = fopen(filename, "r");
      char read_line[100];

      // skip the next two lines
      fgets(read_line, sizeof(read_line), read_file);
      fgets(read_line, sizeof(read_line), read_file);
      fgets(read_line, sizeof(read_line), read_file);

      char* test = __FILE__ ": in function ‘main’";
      for (int i = 0; i < strlen(test) - 1; ++i)
      {
        ok(read_line[i] == test[i]);
      }

      fgets(read_line, sizeof(read_line), read_file);
      test = __FILE__ ":59 message: THIS IS JUST A TEST!!!";
      for (int i = 0; i < strlen(test) - 1; ++i)
      {
        ok(read_line[i] == test[i]);
      }

      fgets(read_line, sizeof(read_line), read_file);
      test = "  This is a test description! XD";
      for (int i = 0; i < strlen(test) - 1; ++i)
      {
        ok(read_line[i] == test[i]);
      }
    }

    subtest("test debug()")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);

      int rez = KC_INVALID;
      rez = log->debug(log, TEST_DEBUG, __LINE__, __func__);

      ok(rez == KC_SUCCESS);

      destroy_logger(log);
    }

    subtest("test error()")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);

      int rez = KC_INVALID;
      rez = log->error(log, TEST_ERROR, __LINE__, __func__);

      ok(rez == KC_SUCCESS);

      destroy_logger(log);
    }

    subtest("test info()")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);

      int rez = KC_INVALID;
      rez = log->info(log, TEST_INFO, __LINE__, __func__);

      ok(rez == KC_SUCCESS);

      destroy_logger(log);
    }

    subtest("test warning()")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);

      int rez = KC_INVALID;
      rez = log->warning(log, TEST_WARNING, __LINE__, __func__);

      ok(rez == KC_SUCCESS);

      destroy_logger(log);
    }

    subtest("test fatal()")
    {
      struct kc_logger_t* log = new_logger(ex, log_ex, __FILE__);

      // uncomment this if you want to test it, but this function
      // will call "exit(1)", so we can't keep it into the CI

      // int rez = KC_INVALID;
      // rez = log->fatal(log, TEST_FATAL, __LINE__, __func__);

      // ok(rez == KC_SUCCESS);

      destroy_logger(log);
    }

    done_testing();
  }

  return 0;
}
