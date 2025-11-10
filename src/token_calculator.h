// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file token_calculator.h
 * @brief Token counting and calculation utilities
 *
 * Functions for calculating token usage, parsing JSONL session files,
 * and computing percentages relative to context limits.
 */

#ifndef MCCS_TOKEN_CALCULATOR_H
#define MCCS_TOKEN_CALCULATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "result.h"
#include "safe_conv.h"
#include "types_struct.h"

// Result types for token parsing operations
DEFINE_RESULT(ResultTokenCounts, struct token_counts, enum MccsError);
DEFINE_RESULT(ResultVoid, int, enum MccsError);

/**
 * Initialize token_counts structure to all zeros
 *
 * @param tokens    Pointer to token_counts structure to initialize
 */
void init_token_counts(struct token_counts *tokens);

/**
 * Calculate total tokens from all categories
 *
 * @param tokens    Pointer to token_counts structure
 * @return          Result<uint64_t> - Ok with sum of all token categories, Err on overflow
 *
 * @note Following ccusage algorithm: total = input + output + cache_creation + cache_read
 * @error MCCS_ERR_INVALID_JSON if tokens is NULL
 * @error MCCS_ERR_OVERFLOW if addition would overflow uint64_t
 */
ResultU64 calculate_total_tokens(const struct token_counts *tokens);

/**
 * Format token count with K/M/G suffixes for readability
 *
 * @param buf        Output buffer for formatted string
 * @param buf_size   Size of output buffer
 * @param tokens     Token count to format
 *
 * @note Formats: >=1B: "X.XG", >=1M: "X.XM", >=1K: "X.XK", <1K: raw number
 */
void format_tokens(char *buf, size_t buf_size, uint64_t tokens);

/**
 * Calculate percentage of tokens relative to a limit
 *
 * @param tokens    Current token count
 * @param limit     Maximum token limit (e.g., 200000 for context window)
 * @param clamp     If true, cap result at 100%
 * @return          Percentage as uint32_t (0-100 if clamped, or higher if not)
 */
uint32_t calculate_percentage(uint64_t tokens,
                              uint64_t limit,
                              bool clamp);

/**
 * Parse a session JSONL file and sum all token usage
 *
 * @param session_path    Path to JSONL transcript file
 * @return                Result<token_counts> - Ok with token counts or Err with error code
 *
 * @note Reads the transcript file line by line, parsing each line as JSON.
 *       Skips empty lines and invalid JSON entries without failing.
 * @error MCCS_ERR_FILE_NOT_FOUND if file cannot be opened
 */
ResultTokenCounts parse_session_tokens(const char *session_path);

/**
 * Count context tokens from the last assistant message in transcript
 *
 * @param transcript_path    Path to JSONL transcript file
 * @return                   Result<uint64_t> - Ok with context token count or Err with error code
 *
 * @note Following ccusage algorithm: Context = last assistant message's input tokens.
 * @error MCCS_ERR_FILE_NOT_FOUND if file cannot be opened
 */
ResultU64 count_context_tokens(const char *transcript_path);

/**
 * Parse tokens in a single pass through the transcript file (optimized)
 *
 * @param transcript_path    Path to JSONL transcript file
 * @param session_tokens     Output structure for accumulated session token counts (can be NULL)
 * @param context_tokens     Output for context tokens from last assistant message (can be NULL)
 * @return                   Result<void> - Ok(0) if successful or Err with error code
 *
 * @note Combines session token accumulation and context token extraction in one pass.
 *       Either output parameter can be NULL if not needed.
 * @error MCCS_ERR_FILE_NOT_FOUND if file cannot be opened
 */
ResultVoid parse_tokens_single_pass(const char *transcript_path,
                                    struct token_counts *session_tokens,
                                    uint64_t *context_tokens);

#endif /* MCCS_TOKEN_CALCULATOR_H */
