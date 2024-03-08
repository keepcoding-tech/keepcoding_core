// This file is part of keepcoding_core
// ==================================
//
// common.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

const char* kc_error_msg[] =
{
    "Operation in peding by the user.",
    "Operation aborted by the user.",
    "The structure was not initialized.",
    "The reference was not initialized.",
    "Insufficient memory in the heap.",
    "Below minimum value.",
    "Exceeds maximum value.",
    "Attempted division by zero.",
    "Access of an invalid index.",
    "Does not meet the constraints.",
    "Failed to open the specified directory.",
    "The specified file was not found.",
    "Failed to open the specified file.",
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

//---------------------------------------------------------------------------//

int get_kc_error_count()
{
  return (sizeof(kc_error_msg) / sizeof(kc_error_msg[0]));
}

//---------------------------------------------------------------------------//

