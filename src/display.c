// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "display.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "constants.h"
#include "safe_conv.h"
#include "token_calculator.h"

/**
 * Extract the basename from a file path (modifies path in-place)
 *
 * @param path    Path string to extract basename from (modified)
 * @return        Pointer to basename within the path string, or "?" if invalid
 */
static const char *mccs_extract_basename(char *path) {
  if (!path || !path[0]) {
    return UNKNOWN_VALUE;
  }

  // Remove trailing slashes
  char *end = path + strlen(path) - 1;
  while (end > path && *end == '/') {
    *end = '\0';
    --end;
  }

  // Handle root directory case
  if (end == path && *end == '/') {
    return "/";
  }

  // Find the last slash to locate basename
  char *slash = end;
  while (slash > path && *(slash - 1) != '/') {
    --slash;
  }

  return slash;
}

/**
 * Return color from theme based on use_color flag
 *
 * @param use_color    If true, return colored theme; if false, return no-color theme
 * @return             Pointer to appropriate color theme
 */
static inline const struct color_theme *get_colors(bool use_color) {
  return get_theme(use_color);
}

/**
 * Print a visual progress bar with percentage fill
 *
 * @param use_color    Whether to use ANSI colors
 * @param percentage   Percentage value (0-100, or higher if not clamped)
 * @param clamp        If true, cap display at 100%
 * @param bar_color    ANSI color code for filled portion
 */
static void print_progress_bar(bool use_color,
                               uint32_t percentage,
                               bool clamp,
                               const char *bar_color,
                               const char *empty_color_override) {
  const uint32_t bar_width = PROGRESS_BAR_WIDTH;
  uint32_t display_pct = clamp && percentage > 100 ? 100 : percentage;
  uint32_t filled = (display_pct * bar_width) / 100;
  if (filled > bar_width) {
    filled = bar_width;
  }

  if (!bar_color) {
    bar_color = "";
  }

  const struct color_theme *c = get_colors(use_color);
  const char *empty_color = empty_color_override ? empty_color_override : c->progress_empty;

  printf("%s[%s", c->reset, bar_color);
  for (uint32_t i = 0; i < bar_width; i++) {
    if (i < filled) {
      printf(PROGRESS_BAR_FILLED);
    } else {
      printf("%s" PROGRESS_BAR_EMPTY, empty_color);
    }
  }
  printf("%s]", c->reset);
}

void print_token_breakdown(bool use_color,
                           bool use_verbose,
                           const struct token_counts *tokens) {
  if (!tokens) {
    return;
  }

  // Hide display entirely if all token values are zero
  if (tokens->input_tokens == 0 && tokens->output_tokens == 0 &&
      tokens->cache_creation_tokens == 0 && tokens->cache_read_tokens == 0) {
    return;
  }

  char buf_in[32], buf_out[32], buf_cr[32], buf_rd[32];

  format_tokens(buf_in, sizeof(buf_in), tokens->input_tokens);
  format_tokens(buf_out, sizeof(buf_out), tokens->output_tokens);
  format_tokens(buf_cr, sizeof(buf_cr), tokens->cache_creation_tokens);
  format_tokens(buf_rd, sizeof(buf_rd), tokens->cache_read_tokens);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sInput: %s%s%s  Output: %s%s%s  Cache Write: %s%s%s  Cache Read: %s%s%s\n",
           c->reset,
           c->token_input, buf_in, c->reset,
           c->token_output, buf_out, c->reset,
           c->token_cache_create, buf_cr, c->reset,
           c->token_cache_read, buf_rd, c->reset);
  } else {
    printf("%sIn: %s%s%s  Out: %s%s%s  CaWr: %s%s%s  CaRd: %s%s%s\n",
           c->reset,
           c->token_input, buf_in, c->reset,
           c->token_output, buf_out, c->reset,
           c->token_cache_create, buf_cr, c->reset,
           c->token_cache_read, buf_rd, c->reset);
  }
}

void print_context_percentage(bool use_color,
                              bool use_verbose,
                              uint64_t context_tokens,
                              bool clamp) {
  uint32_t percentage = calculate_percentage(context_tokens, DEFAULT_TOKEN_LIMIT, clamp);
  char buf_tokens[32], buf_limit[32];
  format_tokens(buf_tokens, sizeof(buf_tokens), context_tokens);
  format_tokens(buf_limit, sizeof(buf_limit), DEFAULT_TOKEN_LIMIT);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sContext   ", c->reset);
    print_progress_bar(use_color,
                       percentage,
                       clamp,
                       c->progress_ctx,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %7u%% (%s used / %s limit)\n", percentage, buf_tokens, buf_limit);
  } else {
    printf("%sCtx%s ", c->label, c->reset);
    print_progress_bar(use_color,
                       percentage,
                       clamp,
                       c->progress_ctx,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %s\n", buf_tokens);
  }
}

void print_session_total(bool use_color,
                         bool use_verbose,
                         uint64_t total_tokens,
                         bool clamp) {
  // Hide display entirely if no tokens
  if (total_tokens == 0) {
    return;
  }

  uint32_t percentage = calculate_percentage(total_tokens, DEFAULT_TOKEN_LIMIT, clamp);
  char buf_total[32], buf_limit[32];
  format_tokens(buf_total, sizeof(buf_total), total_tokens);
  format_tokens(buf_limit, sizeof(buf_limit), DEFAULT_TOKEN_LIMIT);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sSession   ", c->reset);
    print_progress_bar(use_color,
                       percentage,
                       clamp,
                       c->progress_ses,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %7u%% (%s used / %s limit)\n", percentage, buf_total, buf_limit);
  } else {
    printf("%sSes%s ", c->label, c->reset);
    print_progress_bar(use_color,
                       percentage,
                       clamp,
                       c->progress_ses,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %s\n", buf_total);
  }
}

void print_cache_efficiency(bool use_color,
                            bool use_verbose,
                            const struct token_counts *tokens) {
  if (!tokens) {
    return;
  }

  uint64_t cache_read = tokens->cache_read_tokens;
  uint64_t cache_creation = tokens->cache_creation_tokens;

  // Use overflow-checked addition for cache_total
  ResultU64 cache_total_result = safe_add_uint64(cache_read, cache_creation);
  uint64_t cache_total = IS_OK(cache_total_result) ? UNWRAP_OK(cache_total_result) : UINT64_MAX;

  // Hide display entirely if no cache tokens
  if (cache_total == 0) {
    return;
  }

  // Calculate efficiency percentage: reads / total with overflow check
  uint32_t percentage = 0;
  if (cache_total > 0) {
    ResultU64 product_result = safe_mul_uint64(cache_read, 100);
    if (IS_OK(product_result)) {
      uint64_t product = UNWRAP_OK(product_result);
      uint64_t pct = product / cache_total;
      percentage = (uint32_t)(pct > UINT32_MAX ? UINT32_MAX : pct);
    }
  }

  char buf_read[32], buf_total[32];
  format_tokens(buf_read, sizeof(buf_read), cache_read);
  format_tokens(buf_total, sizeof(buf_total), cache_total);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sCache     ", c->reset);
    print_progress_bar(use_color,
                       percentage,
                       false,
                       c->progress_cache,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %7u%% (%s read / %s total)\n", percentage, buf_read, buf_total);
  } else {
    printf("%sCef%s ", c->label, c->reset);
    print_progress_bar(use_color,
                       percentage,
                       false,
                       c->progress_cache,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %s/%s\n", buf_read, buf_total);
  }
}

void print_api_time_ratio(bool use_color,
                          bool use_verbose,
                          uint32_t api_ms,
                          uint32_t total_ms) {
  // Calculate percentage: API time / total time
  uint32_t percentage = 0;
  if (total_ms > 0) {
    uint64_t product = (uint64_t)api_ms * 100;
    percentage = (uint32_t)(product / total_ms);
    if (percentage > 100) {
      percentage = 100;
    }
  }

  // Convert milliseconds to seconds for display
  double api_s = api_ms / MS_PER_SECOND;
  double total_s = total_ms / MS_PER_SECOND;

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sAPI Time  ", c->reset);
    print_progress_bar(use_color,
                       percentage,
                       false,
                       c->progress_api_time,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %7u%% (%.1fs API / %.1fs total)\n", percentage, api_s, total_s);
  } else {
    printf("%sAPI%s ", c->label, c->reset);
    print_progress_bar(use_color,
                       percentage,
                       false,
                       c->progress_api_time,
                       use_color ? ANSI_CTX_EMPTY : NULL);
    printf(" %.1fs/%.1fs\n", api_s, total_s);
  }
}

void print_lines_ratio(bool use_color,
                       bool use_verbose,
                       uint32_t added,
                       uint32_t removed) {
  const uint32_t bar_width = PROGRESS_BAR_WIDTH;

  // Use safe addition for total_changes
  ResultU32 total_changes_result = safe_add_uint32(added, removed);
  uint32_t total_changes = IS_OK(total_changes_result) ? UNWRAP_OK(total_changes_result) : UINT32_MAX;

  // Hide display entirely if no changes
  if (total_changes == 0) {
    return;
  }

  // Calculate percentages and bar segments
  uint32_t added_pct = 0;
  uint32_t removed_pct = 0;
  uint32_t added_width = 0;
  uint32_t removed_width = 0;

  if (total_changes > 0) {
    // Safe calculation for percentage
    ResultU64 product_result = safe_mul_uint64((uint64_t)added, 100);
    if (IS_OK(product_result)) {
      uint64_t product = UNWRAP_OK(product_result);
      added_pct = (uint32_t)(product / total_changes);
      if (added_pct > 100) added_pct = 100;
    }
    removed_pct = 100 - added_pct;

    // Safe calculation for bar widths
    ResultU64 width_product_result = safe_mul_uint64((uint64_t)added, bar_width);
    if (IS_OK(width_product_result)) {
      uint64_t width_product = UNWRAP_OK(width_product_result);
      added_width = (uint32_t)(width_product / total_changes);
      if (added_width > bar_width) added_width = bar_width;
    }
    removed_width = bar_width - added_width;
  }

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sLines    %s [%s", c->reset, c->reset, c->lines_added);
    for (uint32_t i = 0; i < added_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->lines_removed);
    for (uint32_t i = 0; i < removed_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] %3u%%/%u%% (%" PRIu32 " added / %" PRIu32 " removed)\n",
           c->reset, added_pct, removed_pct, added, removed);
  } else {
    printf("%sLin%s [%s", c->label, c->reset, c->lines_added);
    for (uint32_t i = 0; i < added_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->lines_removed);
    for (uint32_t i = 0; i < removed_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] +%" PRIu32 "/-%" PRIu32 "\n",
           c->reset, added, removed);
  }
}

void print_input_output_ratio(bool use_color,
                              bool use_verbose,
                              const struct token_counts *tokens) {
  if (!tokens) {
    return;
  }

  const uint32_t bar_width = PROGRESS_BAR_WIDTH;
  uint64_t input = tokens->input_tokens;
  uint64_t output = tokens->output_tokens;

  // Use overflow-checked addition for total
  ResultU64 total_result = safe_add_uint64(input, output);
  uint64_t total = IS_OK(total_result) ? UNWRAP_OK(total_result) : UINT64_MAX;

  // Hide display entirely if no tokens
  if (total == 0) {
    return;
  }

  // Calculate percentages and bar segments
  uint32_t input_pct = 0;
  uint32_t output_pct = 0;
  uint32_t input_width = 0;
  uint32_t output_width = 0;

  if (total > 0) {
    // Safe multiplication for percentage calculation
    ResultU64 pct_product_result = safe_mul_uint64(input, 100);
    if (IS_OK(pct_product_result)) {
      uint64_t pct_product = UNWRAP_OK(pct_product_result);
      uint64_t input_pct_64 = pct_product / total;
      input_pct = (uint32_t)(input_pct_64 > UINT32_MAX ? UINT32_MAX : input_pct_64);
      if (input_pct > 100) input_pct = 100;
    }
    output_pct = 100 - input_pct;

    // Safe multiplication for bar width calculation
    ResultU64 width_product_result = safe_mul_uint64(input, bar_width);
    if (IS_OK(width_product_result)) {
      uint64_t width_product = UNWRAP_OK(width_product_result);
      uint64_t input_width_64 = width_product / total;
      input_width = (uint32_t)(input_width_64 > bar_width ? bar_width : input_width_64);
    }
    output_width = bar_width - input_width;
  }

  char buf_input[32], buf_output[32];
  format_tokens(buf_input, sizeof(buf_input), input);
  format_tokens(buf_output, sizeof(buf_output), output);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sTokens IO%s [%s", c->reset, c->reset, c->token_input);
    for (uint32_t i = 0; i < input_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->token_output);
    for (uint32_t i = 0; i < output_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] %3u%%/%u%% (%s input / %s output)\n",
           c->reset, input_pct, output_pct, buf_input, buf_output);
  } else {
    printf("%sTIO%s [%s", c->label, c->reset, c->token_input);
    for (uint32_t i = 0; i < input_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->token_output);
    for (uint32_t i = 0; i < output_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] %s/%s\n", c->reset, buf_input, buf_output);
  }
}

void print_cache_write_read_ratio(bool use_color,
                                  bool use_verbose,
                                  const struct token_counts *tokens) {
  if (!tokens) {
    return;
  }

  const uint32_t bar_width = PROGRESS_BAR_WIDTH;
  uint64_t cache_write = tokens->cache_creation_tokens;
  uint64_t cache_read = tokens->cache_read_tokens;

  // Use overflow-checked addition for total
  ResultU64 total_result = safe_add_uint64(cache_write, cache_read);
  uint64_t total = IS_OK(total_result) ? UNWRAP_OK(total_result) : UINT64_MAX;

  // Hide display entirely if no cache tokens
  if (total == 0) {
    return;
  }

  // Calculate percentages and bar segments
  uint32_t write_pct = 0;
  uint32_t read_pct = 0;
  uint32_t write_width = 0;
  uint32_t read_width = 0;

  if (total > 0) {
    // Safe multiplication for percentage calculation
    ResultU64 pct_product_result = safe_mul_uint64(cache_write, 100);
    if (IS_OK(pct_product_result)) {
      uint64_t pct_product = UNWRAP_OK(pct_product_result);
      uint64_t write_pct_64 = pct_product / total;
      write_pct = (uint32_t)(write_pct_64 > UINT32_MAX ? UINT32_MAX : write_pct_64);
      if (write_pct > 100) write_pct = 100;
    }
    read_pct = 100 - write_pct;

    // Safe multiplication for bar width calculation
    ResultU64 width_product_result = safe_mul_uint64(cache_write, bar_width);
    if (IS_OK(width_product_result)) {
      uint64_t width_product = UNWRAP_OK(width_product_result);
      uint64_t write_width_64 = width_product / total;
      write_width = (uint32_t)(write_width_64 > bar_width ? bar_width : write_width_64);
    }
    read_width = bar_width - write_width;
  }

  char buf_write[32], buf_read[32];
  format_tokens(buf_write, sizeof(buf_write), cache_write);
  format_tokens(buf_read, sizeof(buf_read), cache_read);

  const struct color_theme *c = get_colors(use_color);

  if (use_verbose) {
    printf("%sCache RW %s [%s", c->reset, c->reset, c->token_cache_create);
    for (uint32_t i = 0; i < write_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->token_cache_read);
    for (uint32_t i = 0; i < read_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] %3u%%/%u%% (%s write / %s read)\n",
           c->reset, write_pct, read_pct, buf_write, buf_read);
  } else {
    printf("%sCWR%s [%s", c->label, c->reset, c->token_cache_create);
    for (uint32_t i = 0; i < write_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s", c->token_cache_read);
    for (uint32_t i = 0; i < read_width; i++) {
      printf(PROGRESS_BAR_FILLED);
    }
    printf("%s] %s/%s\n", c->reset, buf_write, buf_read);
  }
}

void print_mccs_status_line(bool use_color,
                            bool use_verbose,
                            const struct mccs_status *status,
                            bool simple) {
  if (!status) {
    return;
  }

  const struct mccs_string_refs *refs = &status->string_refs;
  const struct mccs_counters *counters = &status->counters;
  const struct mccs_buffers *buffers = &status->buffers;

  char cwd_copy[BUF_PATH_SIZE];
  char proj_copy[BUF_PATH_SIZE];

  const struct color_theme *c = get_colors(use_color);

  // Calculate cost early so it's available for simple mode
  double cost = isnan(counters->cost_usd) ? ZERO_VALUE : counters->cost_usd;

  // Simple status line mode - show only Model/Version/Directory/Cost
  if (simple) {
    const char *cwd_display = refs->cwd;
    if (refs->cwd == buffers->buf_cwd) {
      (void)snprintf(cwd_copy, sizeof(cwd_copy), "%s", buffers->buf_cwd);
      cwd_display = mccs_extract_basename(cwd_copy);
    }

    if (use_verbose) {
      printf("%s%sModel:%s %s%s%s (%s%s%s) %s|%s %sVersion:%s %s%s%s %s|%s %sCost:%s %s$%.4f%s %s|%s %sDirectory:%s %s%s%s\n",
             c->reset, c->reset, c->reset,
             c->model_name, refs->model_name, c->reset,
             c->model_id, refs->model_id, c->reset,
             c->reset, c->reset,
             c->reset, c->reset,
             c->version, refs->version, c->reset,
             c->reset, c->reset,
             c->reset, c->reset,
             c->cost, cost, c->reset,
             c->reset, c->reset,
             c->reset, c->reset,
             c->dir, cwd_display, c->reset);
    } else {
      printf("%s%s%s%s (%s%s%s) | %s%s%s | %s$%.4f%s | %s%s%s\n",
             c->reset,
             c->model_name, refs->model_name, c->reset,
             c->model_id, refs->model_id, c->reset,
             c->version, refs->version, c->reset,
             c->cost, cost, c->reset,
             c->dir, cwd_display, c->reset);
    }
    return;
  }

  double dur_s = counters->duration_ms / MS_PER_SECOND;
  double api_s = counters->api_ms / MS_PER_SECOND;

  uint32_t added = counters->lines_added;
  uint32_t removed = counters->lines_removed;

  const char *cwd_display = refs->cwd;
  const char *proj_display = refs->project_dir;

  if (refs->cwd == buffers->buf_cwd) {
    (void)snprintf(cwd_copy,
                   sizeof(cwd_copy),
                   "%s",
                   buffers->buf_cwd);
    cwd_display = mccs_extract_basename(cwd_copy);
  }

  if (refs->project_dir == buffers->buf_project) {
    (void)snprintf(proj_copy,
                   sizeof(proj_copy),
                   "%s",
                   buffers->buf_project);
    proj_display = mccs_extract_basename(proj_copy);
  }

  printf("%s", c->reset);

  const char *badge_text = counters->exceeds_200k_tokens ? ">200k" : "<200k";
  const char *c_badge = counters->exceeds_200k_tokens ? c->badge_over : c->badge_under;

  if (strcmp(cwd_display, proj_display) == 0) {
    if (use_verbose) {
      printf(FMT_STATUS_COMPACT_VERBOSE,
             c->reset,                                  // initial reset
             c->reset, c->reset,                        // "Model:" label
             c->model_name, refs->model_name, c->reset, // model name value
             c->model_id, refs->model_id, c->reset,     // model id value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Version:" label
             c->version, refs->version, c->reset,       // version value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Directory:" label
             c->dir, cwd_display, c->reset,             // directory value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Cost:" label
             c->cost, cost, c->reset,                   // cost value
             c->reset, c->reset,                        // "Tokens:" label
             c_badge, badge_text, c->reset,             // badge value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Total:" label
             c->time_total, dur_s, c->reset,            // total time value
             c->reset, c->reset,                        // "API:" label
             c->time_api, api_s, c->reset,              // API time value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Lines:" label
             c->lines_added, added, c->reset,           // lines added value
             c->lines_removed, removed, c->reset);      // lines removed value
    } else {
      printf(FMT_STATUS_COMPACT_PLAIN,
             c->reset,                                  // initial reset
             c->model_name, refs->model_name, c->reset, // model name value
             c->model_id, refs->model_id, c->reset,     // model id value
             c->version, refs->version, c->reset,       // version value
             c->dir, cwd_display, c->reset,             // directory value
             c->cost, cost, c->reset,                   // cost value
             c_badge, badge_text, c->reset,             // badge value
             c->time_total, dur_s, c->reset,            // total time value
             c->time_api, api_s, c->reset,              // API time value
             c->lines_added, added, c->reset,           // lines added value
             c->lines_removed, removed, c->reset);      // lines removed value
    }
  } else {
    if (use_verbose) {
      printf(FMT_STATUS_EXTENDED_VERBOSE,
             c->reset,                                  // initial reset
             c->reset, c->reset,                        // "Model:" label
             c->model_name, refs->model_name, c->reset, // model name value
             c->model_id, refs->model_id, c->reset,     // model id value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Version:" label
             c->version, refs->version, c->reset,       // version value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Directory:" label
             c->dir, cwd_display, c->reset,             // directory value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Project:" label
             c->dir, proj_display, c->reset,            // project value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Cost:" label
             c->cost, cost, c->reset,                   // cost value
             c->reset, c->reset,                        // "Tokens:" label
             c_badge, badge_text, c->reset,             // badge value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Total:" label
             c->time_total, dur_s, c->reset,            // total time value
             c->reset, c->reset,                        // "API:" label
             c->time_api, api_s, c->reset,              // API time value
             c->reset, c->reset,                        // separator "|"
             c->reset, c->reset,                        // "Lines:" label
             c->lines_added, added, c->reset,           // lines added value
             c->lines_removed, removed, c->reset);      // lines removed value
    } else {
      printf(FMT_STATUS_EXTENDED_PLAIN,
             c->reset,                                  // initial reset
             c->model_name, refs->model_name, c->reset, // model name value
             c->model_id, refs->model_id, c->reset,     // model id value
             c->version, refs->version, c->reset,       // version value
             c->dir, cwd_display, c->reset,             // directory value
             c->dir, proj_display, c->reset,            // project value
             c->cost, cost, c->reset,                   // cost value
             c_badge, badge_text, c->reset,             // badge value
             c->time_total, dur_s, c->reset,            // total time value
             c->time_api, api_s, c->reset,              // API time value
             c->lines_added, added, c->reset,           // lines added value
             c->lines_removed, removed, c->reset);      // lines removed value
    }
  }
}
