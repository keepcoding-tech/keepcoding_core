// This file is part of keepcoding_core
// ==================================
//
// test.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/test.h"

#include <stdlib.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

bool check_ok    (bool condition);
void check_skip  (bool condition);

//--- MARK: GLOBAL VARIABLES ------------------------------------------------//

int passed = 0;
int failed = 0;
int skiped = 0;

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

