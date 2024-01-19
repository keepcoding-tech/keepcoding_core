// This file is part of keepcoding_core
// ==================================
//
// error.h
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
 * The EXCEPTION enumeration lists common exception types, and the exception and
 * log_exception arrays provide string representations and detailed descriptions
 * for each exception type, aiding in better error reporting and debugging.
 */

#ifndef KC_ERROR_H
#define KC_ERROR_H

enum ERRORS
{
  KC_SUCCESS = 0,             // Success
  KC_PENDING,                 // Pending: operation in progress
  KC_ABORT,                   // Abort: operation aborted by the user
  KC_EMPTY_STRUCTURE,         // Empty structure: the structure was not initialized
  KC_INDEX_OUT_OF_BOUNDS,     // Out of bounds: access of an invalid index
  KC_NULL_REFERENCE,          // Null Reference: the reference was not initialized
  KC_OUT_OF_MEMORY,           // Out of memory: insufficient memory in the heap
  KC_UNDERFLOW,               // Underflow: below minimum value
  KC_OVERFLOW,                // Overflow: exceeds maximum value
  KC_DIVISION_BY_ZERO,        // Division by 0: attempted a division by zero
  KC_INVALID_ARGUMENT,        // Invalid Argument: does not meet the constraints
  KC_FILE_NOT_FOUND,          // File not found: the specified file was not found
  KC_IO_ERROR,                // I/O error: input/output operation failed
  KC_INTERRUPTED_OPERATION,   // Interrupted: the operation was interrupted by an internal/external signal
  KC_TIMEOUT,                 // Timeout: exceeded the specified time limit
  KC_NETWORK_ERROR,           // Network error: network communication or connectivity error
  KC_INVALID_OPERATION,       // Invalid operation: invalid operation in the current context
  KC_FORMAT_ERROR,            // Format error: input does not conform to the expected format
  KC_PARSE_ERROR,             // Parse error: while parsing, an error was encountered
  KC_UNSUPPORTED_FEATURE,     // Unsupported feature: the feature was not implemented yet
  KC_DATA_CORRUPTION,         // Data corruption: data integrity is compromised or corrupted
  KC_RESOURCE_UNAVAILABLE,    // Resource unavailable: resource is not available
  KC_CONFIGURATION_ERROR,     // Configuration error: an error occurred in configuration or settings
  KC_SYSTEM_ERROR,            // System error: an unexpected or system-level error occured
  KC_DEADLOCK,                // Deadlock: a deadlock situation was encountered
  KC_UNHANDLED_EXCEPTION,     // Unhandled exception occurred
  KC_UNDEFINED_BEHAVIOR,      // Undefined behavior was observed or encountered
  KC_BUFFER_OVERFLOW,         // Buffer overflow: data buffer overflow occurred
  KC_BUFFER_UNDERFLOW,        // Buffer underflow: data buffer underflow occurred
  KC_THREAD_ERROR,            // Thread Error: related to thread management or execution
  KC_CONCURRENT_ACCESS,       // Concurrent access: to a resource resulted in an error
  KC_LOST_CONNECTION,         // Lost connection: the connection was lost with a resource or service
  KC_PROTOCOL_ERROR,          // Protocol error: in protocol adherence or communication
  KC_SECURITY_ERROR,          // Security error: security-related or violation
  KC_COMPILATION_ERROR,       // Compilation error: encountered during code compilation
  KC_RECOVERABLE_ERROR,       // Recoverable error occurred
  KC_UNRECOVERABLE_ERROR,     // Unrecoverable error occurred
  KC_FATAL_ERROR,             // Fatal error occurred
  KC_UNKNOWN_ERROR            // Unknown error occurred
};

extern const char* err[];
extern const char* log_err[];

#endif /* KC_ERROR_H */
