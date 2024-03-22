// This file is part of keepcoding_core
// ==================================
//
// common.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/common.h"

const char* kc_error_msg[] =
{
  KC_INVALID_LOG,
  KC_SUCCESS_LOG,

  KC_PENDING_LOG,
  KC_ABORT_LOG,

  KC_EMPTY_STRUCTURE_LOG,
  KC_NULL_REFERENCE_LOG,

  KC_OUT_OF_MEMORY_LOG,
  KC_UNDERFLOW_LOG,
  KC_OVERFLOW_LOG,
  KC_DIVISION_BY_ZERO_LOG,
  KC_INDEX_OUT_OF_BOUNDS_LOG,

  KC_INVALID_ARGUMENT_LOG,

  KC_CANT_OPEN_DIR_LOG,
  KC_FILE_NOT_FOUND_LOG,
  KC_CANNOT_OPEN_FILE_LOG,
  KC_IO_ERROR_LOG,
  KC_FORMAT_ERROR_LOG,
  KC_PARSE_ERROR_LOG,

  KC_INTERRUPTED_OPERATION_LOG,
  KC_TIMEOUT_LOG,

  KC_INVALID_OPERATION_LOG,

  KC_DATA_CORRUPTION_LOG,

  KC_UNSUPPORTED_FEATURE_LOG,

  KC_RESOURCE_UNAVAILABLE_LOG,
  KC_DEADLOCK_LOG,
  KC_LOST_CONNECTION_LOG,

  KC_NETWORK_ERROR_LOG,
  KC_PROTOCOL_ERROR_LOG,

  KC_SECURITY_ERROR_LOG,

  KC_COMPILATION_ERROR_LOG,
  KC_CONFIGURATION_ERROR_LOG,

  KC_SYSTEM_ERROR_LOG,
  KC_THREAD_ERROR_LOG,

  KC_RECOVERABLE_ERROR_LOG,
  KC_UNRECOVERABLE_ERROR_LOG,
  KC_FATAL_LOG_ERROR_LOG,

  KC_UNKNOWN_ERROR_LOG
};

//---------------------------------------------------------------------------//

int get_kc_error_count()
{
  return (sizeof(kc_error_msg) / sizeof(kc_error_msg[0]));
}

//---------------------------------------------------------------------------//

