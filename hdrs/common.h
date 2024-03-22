// This file is part of keepcoding_core
// ==================================
//
// common.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * This header file provides an exception handling framework through
 * enumerations and corresponding arrays of exception descriptions. It
 * encapsulates various types of exceptions that can occur during program
 * execution. Developers can use this framework to handle and report errors in
 * a structured manner.
 *
 * The enumeration lists common exception types, and the err and log_err
 * arrays provide string representations and detailed descriptions
 * for each exception type, aiding in better error reporting and debugging.
 */

#ifndef KC_COMMON_H
#define KC_COMMON_H

//---------------------------------------------------------------------------//

enum
{
  KC_INVALID = -1,          // Invalid: invalid state, the process cannot proceed
  KC_SUCCESS,               // Success: successful completion of the process

  KC_PENDING,               // Pending: operation in progress
  KC_ABORT,                 // Abort: operation aborted by the user

  KC_EMPTY_STRUCTURE,       // Empty structure: the structure was not initialized
  KC_NULL_REFERENCE,        // Null Reference: the reference was not initialized

  KC_OUT_OF_MEMORY,         // Out of memory: insufficient memory in the heap
  KC_UNDERFLOW,             // Underflow: below minimum value
  KC_OVERFLOW,              // Overflow: exceeds maximum value
  KC_DIVISION_BY_ZERO,      // Division by 0: attempted a division by zero
  KC_INDEX_OUT_OF_BOUNDS,   // Out of bounds: access of an invalid index

  KC_INVALID_ARGUMENT,      // Invalid Argument: does not meet the constraints

  KC_CANT_OPEN_DIR,         // Cannot open the specified directory
  KC_FILE_NOT_FOUND,        // File not found: the specified file was not found
  KC_CANNOT_OPEN_FILE,      // Cannot open file: failed to open the specified file
  KC_IO_ERROR,              // I/O error: input/output operation failed
  KC_FORMAT_ERROR,          // Format error: input does not conform to the expected format
  KC_PARSE_ERROR,           // Parse error: while parsing, an error was encountered

  KC_INTERRUPTED_OPERATION, // Interrupted: the operation was interrupted by an internal/external signal
  KC_TIMEOUT,               // Timeout: exceeded the specified time limit

  KC_INVALID_OPERATION,     // Invalid operation: invalid operation in the current context

  KC_DATA_CORRUPTION,       // Data corruption: data integrity is compromised or corrupted

  KC_UNSUPPORTED_FEATURE,   // Unsupported feature: the feature was not implemented yet

  KC_RESOURCE_UNAVAILABLE,  // Resource unavailable: resource is not available
  KC_DEADLOCK,              // Deadlock: a deadlock situation was encountered
  KC_LOST_CONNECTION,       // Lost connection: the connection was lost with a resource or service

  KC_NETWORK_ERROR,         // Network error: network communication or connectivity error
  KC_PROTOCOL_ERROR,        // Protocol error: in protocol adherence or communication

  KC_SECURITY_ERROR,        // Security error: security-related or violation

  KC_COMPILATION_ERROR,     // Compilation error: encountered during code compilation
  KC_CONFIGURATION_ERROR,   // Configuration error: an error occurred in configuration or settings

  KC_SYSTEM_ERROR,          // System error: an unexpected or system-level error occurred
  KC_THREAD_ERROR,          // Thread Error: related to thread management or execution

  KC_RECOVERABLE_ERROR,     // Recoverable error occurred
  KC_UNRECOVERABLE_ERROR,   // Unrecoverable error occurred
  KC_FATAL_LOG_ERROR,       // Fatal error occurred

  KC_UNKNOWN_ERROR          // Unknown error occurred
};

//---------------------------------------------------------------------------//

#define KC_INVALID_LOG                 "Invalid state, the process cannot proceed."
#define KC_SUCCESS_LOG                 "Successful completion of the process."

#define KC_PENDING_LOG                 "Operation in pending by the user."
#define KC_ABORT_LOG                   "Operation aborted by the user."

#define KC_EMPTY_STRUCTURE_LOG         "The structure was not initialized."
#define KC_NULL_REFERENCE_LOG          "The reference was not initialized."

#define KC_OUT_OF_MEMORY_LOG           "Insufficient memory in the heap."
#define KC_UNDERFLOW_LOG               "Below minimum value."
#define KC_OVERFLOW_LOG                "Exceeds maximum value."
#define KC_DIVISION_BY_ZERO_LOG        "Attempted division by zero."
#define KC_INDEX_OUT_OF_BOUNDS_LOG     "Access of an invalid index."

#define KC_INVALID_ARGUMENT_LOG        "Does not meet the constraints."

#define KC_CANT_OPEN_DIR_LOG           "Failed to open the specified directory."
#define KC_FILE_NOT_FOUND_LOG          "The specified file was not found."
#define KC_CANNOT_OPEN_FILE_LOG        "Failed to open the specified file."
#define KC_IO_ERROR_LOG                "Input/output operation failed."
#define KC_FORMAT_ERROR_LOG            "Input does not conform to the expected format."
#define KC_PARSE_ERROR_LOG             "Error encountered while parsing."

#define KC_INTERRUPTED_OPERATION_LOG   "Operation interrupted by an internal/external signal."
#define KC_TIMEOUT_LOG                 "Exceeded the specified time limit."

#define KC_INVALID_OPERATION_LOG       "Invalid operation in the current context."

#define KC_DATA_CORRUPTION_LOG         "Data integrity is compromised or corrupted."

#define KC_UNSUPPORTED_FEATURE_LOG     "Feature was not implemented yet."

#define KC_RESOURCE_UNAVAILABLE_LOG    "Resource is not available."
#define KC_DEADLOCK_LOG                "A deadlock situation was encountered."
#define KC_LOST_CONNECTION_LOG         "Connection was lost with a resource or service."

#define KC_NETWORK_ERROR_LOG           "Network communication or connectivity error."
#define KC_PROTOCOL_ERROR_LOG          "Error in protocol adherence or communication."

#define KC_SECURITY_ERROR_LOG          "Security-related error or violation."

#define KC_COMPILATION_ERROR_LOG       "Error encountered during code compilation."
#define KC_CONFIGURATION_ERROR_LOG     "Error occurred in configuration or settings."

#define KC_SYSTEM_ERROR_LOG            "Unexpected or system-level error occurred."
#define KC_THREAD_ERROR_LOG            "Related to thread management or execution."

#define KC_RECOVERABLE_ERROR_LOG       "Recoverable error occurred."
#define KC_UNRECOVERABLE_ERROR_LOG     "Unrecoverable error occurred."
#define KC_FATAL_LOG_ERROR_LOG         "Fatal error occurred."

#define KC_UNKNOWN_ERROR_LOG           "Unknown error occurred."

//---------------------------------------------------------------------------//

extern const char* kc_error_msg[];

//---------------------------------------------------------------------------//

int get_kc_error_count();

//---------------------------------------------------------------------------//

#endif /* KC_COMMON_H */
