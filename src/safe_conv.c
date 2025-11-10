// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "safe_conv.h"

#include <math.h>
#include <limits.h>

ResultU64 safe_double_to_uint64(double value) {
  if (!isfinite(value)) {
    return ERR(ResultU64, MCCS_ERR_INVALID_CONVERSION);
  }

  if (value < 0.0) {
    return ERR(ResultU64, MCCS_ERR_INVALID_CONVERSION);
  }

  if (value > (double)UINT64_MAX) {
    return ERR(ResultU64, MCCS_ERR_INVALID_CONVERSION);
  }

  return OK(ResultU64, (uint64_t)value);
}

ResultU32 safe_double_to_uint32(double value) {
  if (!isfinite(value)) {
    return ERR(ResultU32, MCCS_ERR_INVALID_CONVERSION);
  }

  if (value < 0.0) {
    return ERR(ResultU32, MCCS_ERR_INVALID_CONVERSION);
  }

  if (value > (double)UINT32_MAX) {
    return ERR(ResultU32, MCCS_ERR_INVALID_CONVERSION);
  }

  return OK(ResultU32, (uint32_t)value);
}

ResultSize safe_ssize_to_size(ssize_t value) {
  if (value < 0) {
    return ERR(ResultSize, MCCS_ERR_INVALID_CONVERSION);
  }

  return OK(ResultSize, (size_t)value);
}

ResultSize safe_off_to_size(off_t value) {
  if (value < 0) {
    return ERR(ResultSize, MCCS_ERR_INVALID_CONVERSION);
  }

  if (sizeof(off_t) > sizeof(size_t)) {
    if ((uintmax_t)value > (uintmax_t)SIZE_MAX) {
      return ERR(ResultSize, MCCS_ERR_INVALID_CONVERSION);
    }
  }

  return OK(ResultSize, (size_t)value);
}

ResultU64 safe_add_uint64(uint64_t a, uint64_t b) {
  if (a > UINT64_MAX - b) {
    return ERR(ResultU64, MCCS_ERR_OVERFLOW);
  }

  return OK(ResultU64, a + b);
}

ResultU64 safe_mul_uint64(uint64_t a, uint64_t b) {
  if (a == 0 || b == 0) {
    return OK(ResultU64, 0);
  }

  if (a > UINT64_MAX / b) {
    return ERR(ResultU64, MCCS_ERR_OVERFLOW);
  }

  return OK(ResultU64, a * b);
}

ResultU32 safe_add_uint32(uint32_t a, uint32_t b) {
  if (a > UINT32_MAX - b) {
    return ERR(ResultU32, MCCS_ERR_OVERFLOW);
  }

  return OK(ResultU32, a + b);
}
