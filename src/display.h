// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file display.h
 * @brief Output formatting and display functions
 *
 * Functions for printing formatted status lines, token breakdowns,
 * progress bars, and other visual elements.
 */

#ifndef MCCS_DISPLAY_H
#define MCCS_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#include "types_struct.h"

/**
 * Print detailed token breakdown by category
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to use long label names
 * @param tokens       Token counts to display
 *
 * @note Output format (non-verbose): In: X.XK  Out: X.XK  CaWr: X.XK  CaRd: X.XK
 * @note Output format (verbose): Input: X.XK  Output: X.XK  Cache Write: X.XK  Cache Read: X.XK
 */
void print_token_breakdown(bool use_color,
                           bool use_verbose,
                           const struct token_counts *tokens);

/**
 * Print context window usage with progress bar
 *
 * @param use_color       Whether to use ANSI colors
 * @param use_verbose     Whether to show verbose labels and percentage
 * @param context_tokens  Current context token count
 * @param clamp           If true, cap percentage display at 100%
 *
 * @note Output format: Ctx: [████░░░░] X.XK/200K (verbose OFF)
 * @note Output format: Context: [████░░░░] X% (X.XM used / 200K limit) (verbose ON)
 */
void print_context_percentage(bool use_color,
                              bool use_verbose,
                              uint64_t context_tokens,
                              bool clamp);

/**
 * Print session total token usage with progress bar
 *
 * @param use_color     Whether to use ANSI colors
 * @param use_verbose   Whether to show verbose labels and percentage
 * @param total_tokens  Total session token count
 * @param clamp         If true, cap percentage display at 100%
 *
 * @note Output format: Ses: [████░░░░] X.XK/200K (verbose OFF)
 * @note Output format: Session: [████░░░░] X% (X.XM used / 200K limit) (verbose ON)
 */
void print_session_total(bool use_color,
                         bool use_verbose,
                         uint64_t total_tokens,
                         bool clamp);

/**
 * Print cache efficiency with progress bar
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show verbose labels and percentage
 * @param tokens       Token counts containing cache read and creation
 *
 * @note Output format: Cef: [████░░░░] X.XM/X.XM (verbose OFF)
 * @note Output format: Cache: [████░░░░] X% (X.XM read / X.XM total) (verbose ON)
 * @note Percentage shows cache_read / (cache_read + cache_creation)
 */
void print_cache_efficiency(bool use_color,
                            bool use_verbose,
                            const struct token_counts *tokens);

/**
 * Print the main status line with all session information
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show field labels
 * @param status       Pointer to loaded status structure
 * @param simple       Whether to show simplified status line (Model/Version/Directory only)
 *
 * @note Format selected based on use_verbose flag and cwd==project_dir comparison.
 */
void print_mccs_status_line(bool use_color,
                            bool use_verbose,
                            const struct mccs_status *status,
                            bool simple);

/**
 * Print API time vs total time ratio with progress bar
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show verbose labels and percentage
 * @param api_ms       API time in milliseconds
 * @param total_ms     Total time in milliseconds
 *
 * @note Output format: API: [████░░░░] 2.3s/5.1s (verbose OFF)
 * @note Output format: API Time: [████░░░░]  45% (2.3s API / 5.1s total) (verbose ON)
 * @note Percentage shows api_ms / total_ms
 */
void print_api_time_ratio(bool use_color,
                          bool use_verbose,
                          uint32_t api_ms,
                          uint32_t total_ms);

/**
 * Print lines added vs removed ratio with dual-color progress bar
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show verbose labels and percentage
 * @param added        Number of lines added
 * @param removed      Number of lines removed
 *
 * @note Output format: Lns: [████████████] +150/-50 (verbose OFF)
 * @note Output format: Lines:    [████████████]  75%/25% (150 added / 50 removed) (verbose ON)
 * @note Bar shows proportions: added (green) and removed (red) segments
 */
void print_lines_ratio(bool use_color,
                       bool use_verbose,
                       uint32_t added,
                       uint32_t removed);

/**
 * Print input vs output tokens ratio with dual-color progress bar
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show verbose labels and percentage
 * @param tokens       Token counts containing input and output tokens
 *
 * @note Output format: Tio: [████████░░░░░░░░░░░░] 4.5K/1.9K (verbose OFF)
 * @note Output format: Tokens IO:[████████░░░░░░░░░░░░]  70%/30% (4.5K input / 1.9K output) (verbose ON)
 * @note Bar shows proportions: input (cyan) and output (green) segments
 */
void print_input_output_ratio(bool use_color,
                              bool use_verbose,
                              const struct token_counts *tokens);

/**
 * Print cache write vs read tokens ratio with dual-color progress bar
 *
 * @param use_color    Whether to use ANSI colors
 * @param use_verbose  Whether to show verbose labels and percentage
 * @param tokens       Token counts containing cache creation and read tokens
 *
 * @note Output format: Cwr: [████████████████░░░░] 3.5K/800 (verbose OFF)
 * @note Output format: Cache RW: [████████████████░░░░]  81%/19% (3.5K write / 800 read) (verbose ON)
 * @note Bar shows proportions: cache write (yellow) and cache read (orchid) segments
 */
void print_cache_write_read_ratio(bool use_color,
                                  bool use_verbose,
                                  const struct token_counts *tokens);

#endif /* MCCS_DISPLAY_H */
