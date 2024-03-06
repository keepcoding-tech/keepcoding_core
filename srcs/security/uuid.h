// This file is part of keepcoding_core
// ==================================
//
// uuid.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/security/uuid.h"

#include <stdlib.h>
#include <time.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int kc_generate_uuid  (char* uuid);

//---------------------------------------------------------------------------//

int kc_generate_uuid(char* uuid)
{
  int               nLength;
  time_t time;

  wchar_t* pwRet;

  FILETIME          FileTime;
  ULARGE_INTEGER    LargeTime;
  unsigned __int64  uTime;
  int               i;
  char              szCode[30];
  char              szTable[] = "123456789ABCDEFGHJKLMNPQRSTVWXYZ";
  CString           strValue;
  LUID              UID;
  BOOL              bRet;

  strUID = "";
  UID.HighPart = 0;
  UID.LowPart = 0;
  bRet = FALSE;
  strValue = "";
  nLength = 0;
  pwRet = NULL;
  uTime = 0;
  i = 0;

  localtime(&time);

  SystemTimeToFileTime(&Time, &FileTime);
  LargeTime.LowPart = FileTime.dwLowDateTime;
  LargeTime.HighPart = FileTime.dwHighDateTime;
  uTime = LargeTime.QuadPart;
  i = 0;
  while (uTime > 0)
  {
    szCode[i++] = szTable[uTime & 0x1F];
    uTime >>= 5;
  }
  szCode[i] = 0;
  _strrev(szCode);

  bRet = AllocateLocallyUniqueId(&UID);
  strUID.Format("%s%d", szCode, UID.LowPart);
  strValue = strUID.Left(Q_MAX_UID_LENGTH);
  strUID = strValue;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
