// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file safe_conv.h
 * @brief Safe type conversion and overflow-checked arithmetic utilities
 *
 * Provides runtime-checked type conversions and arithmetic operations
 * to prevent undefined behavior from overflows and invalid casts.
 * All functions return Result types for explicit error handling.
 */

#ifndef MCCS_SAFE_CONV_H
#define MCCS_SAFE_CONV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "result.h"

// Result type definitions for safe conversions
DEFINE_RESULT(ResultU64, uint64_t, enum MccsError);
DEFINE_RESULT(ResultU32, uint32_t, enum MccsError);
DEFINE_RESULT(ResultSize, size_t, enum MccsError);

/**
 * Safely convert double to uint64_t with range validation
 *
 * @param value      Double value to convert
 * @return           Result<uint64_t> - Ok with converted value or Err with error code
 *
 * @note Rejects: negative values, values > UINT64_MAX, NaN, infinity
 * @error MCCS_ERR_INVALID_CONVERSION for non-finite or out-of-range values
 */
ResultU64 safe_double_to_uint64(double value);

/**
 * Safely convert double to uint32_t with range validation
 *
 * @param value      Double value to convert
 * @return           Result<uint32_t> - Ok with converted value or Err with error code
 *
 * @note Rejects: negative values, values > UINT32_MAX, NaN, infinity
 * @error MCCS_ERR_INVALID_CONVERSION for non-finite or out-of-range values
 */
ResultU32 safe_double_to_uint32(double value);

/**
 * Safely convert ssize_t to size_t
 *
 * @param value      Signed size value to convert
 * @return           Result<size_t> - Ok with converted value or Err with error code
 * @error MCCS_ERR_INVALID_CONVERSION for negative values
 */
ResultSize safe_ssize_to_size(ssize_t value);

/**
 * Safely convert off_t to size_t with range validation
 *
 * @param value      File offset value to convert
 * @return           Result<size_t> - Ok with converted value or Err with error code
 * @error MCCS_ERR_INVALID_CONVERSION for negative or out-of-range values
 */
ResultSize safe_off_to_size(off_t value);

/**
 * Overflow-checked addition: a + b
 *
 * @param a          First operand
 * @param b          Second operand
 * @return           Result<uint64_t> - Ok with sum or Err with error code
 * @error MCCS_ERR_OVERFLOW on arithmetic overflow
 */
ResultU64 safe_add_uint64(uint64_t a, uint64_t b);

/**
 * Overflow-checked multiplication: a * b
 *
 * @param a          First operand
 * @param b          Second operand
 * @return           Result<uint64_t> - Ok with product or Err with error code
 * @error MCCS_ERR_OVERFLOW on arithmetic overflow
 */
ResultU64 safe_mul_uint64(uint64_t a, uint64_t b);

/**
 * Overflow-checked addition: a + b (uint32_t variant)
 *
 * @param a          First operand
 * @param b          Second operand
 * @return           Result<uint32_t> - Ok with sum or Err with error code
 * @error MCCS_ERR_OVERFLOW on arithmetic overflow
 */
ResultU32 safe_add_uint32(uint32_t a, uint32_t b);

#endif /* MCCS_SAFE_CONV_H */
