// This file is part of keepcoding_core
// ==================================
//
// common.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

const char* err[] =
{
  "PENDING",
  "ABORT",
  "EMPTY_STRUCTURE",
  "NULL_REFERENCE",
  "OUT_OF_MEMORY",
  "UNDERFLOW",
  "OVERFLOW",
  "DIVISION_BY_ZERO",
  "INDEX_OUT_OF_BOUNDS",
  "INVALID_ARGUMENT",
  "FILE_NOT_FOUND",
  "IO_ERROR",
  "FORMAT_ERROR",
  "PARSE_ERROR",
  "INTERRUPTED_OPERATION",
  "TIMEOUT",
  "INVALID_OPERATION",
  "DATA_CORRUPTION",
  "UNSUPPORTED_FEATURE",
  "RESOURCE_UNAVAILABLE",
  "DEADLOCK",
  "LOST_CONNECTION",
  "NETWORK_ERROR",
  "PROTOCOL_ERROR",
  "SECURITY_ERROR",
  "COMPILATION_ERROR",
  "CONFIGURATION_ERROR",
  "SYSTEM_ERROR",
  "THREAD_ERROR",
  "RECOVERABLE_ERROR",
  "UNRECOVERABLE_ERROR",
  "FATAL_ERROR",
  "UNKNOWN_ERROR"
};

const char* log_err[] =
{
  "Operation is in progress.",
  "Operation aborted by the user.",
  "The structure was not initialized.",
  "The reference was not initialized.",
  "Insufficient memory in the heap.",
  "Below minimum value.",
  "Exceeds maximum value.",
  "Attempted division by zero.",
  "Access of an invalid index.",
  "Does not meet the constraints.",
  "The specified file was not found.",
  "Input/output operation failed.",
  "Input does not conform to the expected format.",
  "Error encountered while parsing.",
  "Operation interrupted by an internal/external signal.",
  "Exceeded the specified time limit.",
  "Invalid operation in the current context.",
  "Data integrity is compromised or corrupted.",
  "Feature was not implemented yet.",
  "Resource is not available.",
  "A deadlock situation was encountered.",
  "Connection was lost with a resource or service.",
  "Network communication or connectivity error.",
  "Error in protocol adherence or communication.",
  "Security-related error or violation.",
  "Error encountered during code compilation.",
  "Error occurred in configuration or settings.",
  "Unexpected or system-level error occurred.",
  "Related to thread management or execution.",
  "Recoverable error occurred.",
  "Unrecoverable error occurred.",
  "Fatal error occurred.",
  "Unknown error occurred."
};
