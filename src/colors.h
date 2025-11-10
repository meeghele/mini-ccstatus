// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file colors.h
 * @brief Color palette and theme definitions for mini-ccstatus
 *
 * Separates low-level ANSI escape codes (palette) from semantic color
 * assignments (theme). Makes it easy to create alternate themes or disable colors.
 */

#ifndef MCCS_COLORS_H
#define MCCS_COLORS_H

#include <stdbool.h>

/**
 * ANSI 256-color escape codes - single source of truth
 * These macros are used by both palette and theme structures
 */
#define ANSI_RED "\033[1m\033[38;5;197m"          // Monokai pink
#define ANSI_RED_MUTED "\033[1m\033[38;5;161m"    // Monokai muted crimson
#define ANSI_GREEN "\033[1m\033[38;5;148m"        // Monokai lime
#define ANSI_CYAN "\033[1m\033[38;5;81m"          // Monokai aqua
#define ANSI_DARK_CYAN "\033[1m\033[38;5;68m"     // Monokai muted aqua
#define ANSI_YELLOW "\033[1m\033[38;5;186m"       // Monokai cream
#define ANSI_DARK_YELLOW "\033[1m\033[38;5;179m"  // Monokai aged cream
#define ANSI_PURPLE "\033[1m\033[38;5;141m"       // Monokai violet
#define ANSI_LIGHT_PURPLE "\033[1m\033[38;5;104m" // Monokai soft indigo
#define ANSI_ORANGE "\033[1m\033[38;5;208m"       // Monokai orange
#define ANSI_ORCHID "\033[1m\033[38;5;176m"       // Monokai mauve
#define ANSI_ORCHID_SOFT "\033[1m\033[38;5;139m"  // Monokai soft mauve
#define ANSI_LAVENDER "\033[1m\033[38;5;189m"     // Monokai lavender
#define ANSI_WHITE "\033[1m\033[38;5;231m"        // Monokai foreground
#define ANSI_DARK_GRAY "\033[1m\033[38;5;236m"    // Progress background gray
#define ANSI_STEEL_BLUE "\033[1m\033[38;5;60m"    // Context blue
#define ANSI_CTX_EMPTY "\033[1m\033[38;5;233m"    // Context background
#define ANSI_RESET "\033[0m"                      // Reset attributes
#define ANSI_NONE ""                              // Empty (no-color mode)

/**
 * Color theme: semantic color assignments
 * Maps UI elements to palette colors for consistent styling
 */
struct color_theme {
  const char *label;              // Field labels (e.g., "Model:", "Version:")
  const char *model_name;         // Model display name
  const char *model_id;           // Model ID string
  const char *version;            // Version string
  const char *dir;                // Directory/path display
  const char *cost;               // Cost display
  const char *time_total;         // Total duration
  const char *time_api;           // API time
  const char *lines_added;        // Added lines count
  const char *lines_removed;      // Removed lines count
  const char *badge_under;        // <200k token badge
  const char *badge_over;         // >200k token badge
  const char *token_input;        // Input tokens
  const char *token_output;       // Output tokens
  const char *token_cache_create; // Cache write tokens
  const char *token_cache_read;   // Cache read tokens
  const char *progress_empty;     // Empty progress bar segments
  const char *progress_ctx;       // Context window progress bar
  const char *progress_ses;       // Session total progress bar
  const char *progress_cache;     // Cache efficiency progress bar
  const char *progress_api_time;  // API time ratio progress bar
  const char *reset;              // Reset to default
};

/* Default color theme using full-color palette */
static const struct color_theme theme_color = {
    .label = ANSI_RESET,
    .model_name = ANSI_PURPLE,
    .model_id = ANSI_LIGHT_PURPLE,
    .version = ANSI_ORANGE,
    .dir = ANSI_CYAN,
    .cost = ANSI_YELLOW,
    .time_total = ANSI_ORCHID,
    .time_api = ANSI_LAVENDER,
    .lines_added = ANSI_GREEN,
    .lines_removed = ANSI_RED_MUTED,
    .badge_under = ANSI_GREEN,
    .badge_over = ANSI_RED,
    .token_input = ANSI_CYAN,
    .token_output = ANSI_DARK_CYAN,
    .token_cache_create = ANSI_YELLOW,
    .token_cache_read = ANSI_DARK_YELLOW,
    .progress_empty = ANSI_CTX_EMPTY,
    .progress_ctx = ANSI_STEEL_BLUE,
    .progress_ses = ANSI_LIGHT_PURPLE,
    .progress_cache = ANSI_ORCHID_SOFT,
    .progress_api_time = ANSI_STEEL_BLUE,
    .reset = ANSI_RESET,
};

/* No-color theme (all empty strings) */
static const struct color_theme theme_none = {
    .label = ANSI_NONE,
    .model_name = ANSI_NONE,
    .model_id = ANSI_NONE,
    .version = ANSI_NONE,
    .dir = ANSI_NONE,
    .cost = ANSI_NONE,
    .time_total = ANSI_NONE,
    .time_api = ANSI_NONE,
    .lines_added = ANSI_NONE,
    .lines_removed = ANSI_NONE,
    .badge_under = ANSI_NONE,
    .badge_over = ANSI_NONE,
    .token_input = ANSI_NONE,
    .token_output = ANSI_NONE,
    .token_cache_create = ANSI_NONE,
    .token_cache_read = ANSI_NONE,
    .progress_empty = ANSI_NONE,
    .progress_ctx = ANSI_NONE,
    .progress_ses = ANSI_NONE,
    .progress_cache = ANSI_NONE,
    .progress_api_time = ANSI_NONE,
    .reset = ANSI_NONE,
};

/**
 * Get the appropriate color theme based on use_color flag
 *
 * @param use_color    If true, return colored theme; otherwise no-color theme
 * @return             Pointer to the selected theme
 */
static inline const struct color_theme *get_theme(bool use_color) {
  return use_color ? &theme_color : &theme_none;
}

#endif /* MCCS_COLORS_H */
