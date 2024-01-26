// This file is part of keepcoding_core
// ==================================
//
// test.c
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/test.h"

#include <stdlib.h>
#include <stdbool.h>

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

bool check_ok    (bool condition);
void check_skip  (bool condition);

//---------------------------------------------------------------------------//

int passed = 0;
int failed = 0;
int skiped = 0;

bool skiping = false;

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
