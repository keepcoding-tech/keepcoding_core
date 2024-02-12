// This file is part of keepcoding_core
// ==================================
//
// test.h
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

#ifndef KC_TEST_H
#define KC_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool check_ok(bool condition);
void check_skip(bool condition);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

extern int passed;
extern int failed;
extern int skiped;

extern bool skiping;

//---------------------------------------------------------------------------//

#define testgroup(description)                                                \
  printf("\n----- FILE > %s %s \n\n", __FILE__, #description);

//---------------------------------------------------------------------------//

#define subtest(description)                                                  \
  printf(" SUBTEST | %s \n", description);                                    \

//---------------------------------------------------------------------------//

#define ok(condition)                                                         \
  if (skiping == true)                                                        \
  {                                                                           \
    skiping = false;                                                          \
    printf("    SKIP | %s \n", #condition);                                   \
  }                                                                           \
  else if (check_ok(condition) == false)                                      \
  {                                                                           \
    printf("    FAIL |- %s \n", #condition);                                  \
    printf("         |- in file %s:%d \n", __FILE__, __LINE__);               \
  }

//---------------------------------------------------------------------------//

#define skip(condition)                                                       \
  check_skip(condition);

//---------------------------------------------------------------------------//

#define note(description)                                                     \
  printf("    NOTE : %s \n", description);

//---------------------------------------------------------------------------//

#define done_testing()                                                        \
  printf("\n^^^^^ RSLT > %d FAIL | %d PASS | %d SKIP \n",                     \
      failed, passed, skiped);                                                \
  if (failed > 0)                                                             \
  {                                                                           \
    exit(1);                                                                  \
  }

//---------------------------------------------------------------------------//

bool check_ok(bool condition)
{
  if (condition == false)
  {
    // increment the number of failed tests
    ++failed;
    return false;
  }

  // increment the number of passed tests
  ++passed;
  return true;
}

//---------------------------------------------------------------------------//

void check_skip(bool condition)
{
  if (condition == true)
  {
    // increment the number of skiped tests
    ++skiped;
    skiping = true;
  }
}

//---------------------------------------------------------------------------//

#endif /* KC_TEST_H */
