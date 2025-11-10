// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file result.h
 * @brief Railway-oriented programming Result type for C
 *
 * Provides a Rust-like Result<T, E> type for explicit error handling.
 * Supports monadic operations for composing fallible functions.
 * @endcode
 */

#ifndef MCCS_RESULT_H
#define MCCS_RESULT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Generic Result type generator macro
 * Creates a tagged union for Result<T, E>
 *
 * @param name   Name of the Result type to create
 * @param T      Type for successful value
 * @param E      Type for error value
 */
#define DEFINE_RESULT(name, T, E) \
  typedef struct {                \
    bool is_ok;                   \
    union {                       \
      T ok;                       \
      E err;                      \
    } value;                      \
  } name

/**
 * Create a successful Result
 *
 * @param ResultType   The Result type name
 * @param val          The successful value
 */
#define OK(ResultType, val) \
  ((ResultType){.is_ok = true, .value = {.ok = (val)}})

/**
 * Create an error Result
 *
 * @param ResultType   The Result type name
 * @param error        The error value
 */
#define ERR(ResultType, error) \
  ((ResultType){.is_ok = false, .value = {.err = (error)}})

/**
 * Check if Result is Ok
 */
#define IS_OK(result) ((result).is_ok)

/**
 * Check if Result is Err
 */
#define IS_ERR(result) (!(result).is_ok)

/**
 * Extract Ok value (unsafe - check IS_OK first)
 */
#define UNWRAP_OK(result) ((result).value.ok)

/**
 * Extract Err value (unsafe - check IS_ERR first)
 */
#define UNWRAP_ERR(result) ((result).value.err)

/**
 * TRY macro - early return on error
 * If result is error, returns it immediately
 *
 * @param result   The Result to check
 */
#define TRY(result)                   \
  do {                                \
    __typeof__(result) _r = (result); \
    if (IS_ERR(_r))                   \
      return _r;                      \
  } while (0)

/**
 * TRY_VAR macro - early return on error with variable assignment
 * If result is error, returns it immediately
 * Otherwise assigns ok value to variable
 *
 * @param var      Variable to assign ok value to
 * @param result   The Result to check
 */
#define TRY_VAR(var, result)          \
  do {                                \
    __typeof__(result) _r = (result); \
    if (IS_ERR(_r))                   \
      return _r;                      \
    (var) = UNWRAP_OK(_r);            \
  } while (0)

/**
 * MATCH_RESULT - Pattern matching for Result types
 *
 * @param result     The Result to match on
 * @param ok_case    Code block for Ok case (value available as 'ok_val')
 * @param err_case   Code block for Err case (error available as 'err_val')
 */
#define MATCH_RESULT(result, ok_case, err_case)            \
  do {                                                     \
    __typeof__(result) _r = (result);                      \
    if (IS_OK(_r)) {                                       \
      __typeof__(UNWRAP_OK(_r)) ok_val = UNWRAP_OK(_r);    \
      ok_case                                              \
    } else {                                               \
      __typeof__(UNWRAP_ERR(_r)) err_val = UNWRAP_ERR(_r); \
      err_case                                             \
    }                                                      \
  } while (0)

// ============================================================================
// Error Codes
// ============================================================================

/**
 * Error codes for mini-ccstatus.
 */
enum MccsError {
  MCCS_OK = 0,

  // Memory errors
  MCCS_ERR_OUT_OF_MEMORY,
  MCCS_ERR_BUFFER_TOO_SMALL,

  // I/O errors
  MCCS_ERR_FILE_NOT_FOUND,
  MCCS_ERR_IO_ERROR,

  // Parsing errors
  MCCS_ERR_INVALID_JSON,
  MCCS_ERR_MISSING_FIELD,
  MCCS_ERR_TYPE_MISMATCH,
  MCCS_ERR_INVALID_FORMAT,

  // Validation errors
  MCCS_ERR_INVALID_SESSION_ID,
  MCCS_ERR_INVALID_MODEL_ID,
  MCCS_ERR_INVALID_PATH,
  MCCS_ERR_INVALID_PERCENTAGE,
  MCCS_ERR_INVALID_TOKEN_COUNT,
  MCCS_ERR_INVALID_COST,
  MCCS_ERR_INVALID_DURATION,

  // Arithmetic errors
  MCCS_ERR_OVERFLOW,
  MCCS_ERR_UNDERFLOW,
  MCCS_ERR_DIVISION_BY_ZERO,
  MCCS_ERR_INVALID_CONVERSION
};

/**
 * UNWRAP_OR macro - extract Ok value or provide default
 * ACTIVE: Simple and useful for providing fallback values
 *
 * @param result   The Result to unwrap
 * @param default  Default value if error
 */
#define UNWRAP_OR(result, default) \
  (IS_OK(result) ? UNWRAP_OK(result) : (default))

/** Imported but unused
 *
 * #define MAP(result, transform, NewType)            \
 *   (IS_OK(result)                                   \
 *        ? OK(NewType, transform(UNWRAP_OK(result))) \
 *        : ERR(NewType, UNWRAP_ERR(result)))
 *
 * #define AND_THEN(result, f) \
 *   (IS_OK(result) ? f(UNWRAP_OK(result)) : (result))
 *
 * #define OR_ELSE(result, alternative) \
 *   (IS_ERR(result) ? alternative(UNWRAP_ERR(result)) : (result))
 */

#endif /* MCCS_RESULT_H */
