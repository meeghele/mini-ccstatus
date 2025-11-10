// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file debug.h
 * @brief Debug logging and diagnostic utilities
 *
 * Provides DEBUG_LOG macro for conditional debug output.
 * Logging is completely compiled out when -DDEBUG is not set.
 */

#ifndef MCCS_DEBUG_H
#define MCCS_DEBUG_H

#include <stdio.h>

/**
 * Debug logging macro - enabled with -DDEBUG compile flag
 *
 * @param fmt Printf-style format string
 * @param ... Variadic arguments for format string
 */
#ifdef DEBUG
#define DEBUG_LOG(fmt, ...) \
  fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif

#endif /* MCCS_DEBUG_H */
