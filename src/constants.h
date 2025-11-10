// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file constants.h
 * @brief Constants and definitions for mini-ccstatus
 *
 * All compile-time constants including buffer sizes, error codes, color codes,
 * and format strings. Using inline documentation style for compactness.
 */

#ifndef MCCS_CONSTANTS_H
#define MCCS_CONSTANTS_H

#include <inttypes.h>
#include <stdio.h>

#ifndef MCCS_STDOUT
#define MCCS_STDOUT stdout
#endif

#ifndef MCCS_STDERR
#define MCCS_STDERR stdout
#endif

#define ON "on"
#define OFF "off"

/* Buffer size constants - sized based on expected field lengths */
#define BUF_MODEL_NAME_SIZE 64            /* Model display names like "Claude 3.5 Sonnet" */
#define BUF_MODEL_ID_SIZE 128             /* Full model IDs with timestamp suffixes */
#define BUF_PATH_SIZE 256                 /* File system paths (conservative vs PATH_MAX) */
#define BUF_VERSION_SIZE 32               /* Version strings like "4.5.0" */
#define MAX_INPUT_LINE_SIZE (1024 * 1024) /* 1MB limit for JSON input to prevent DoS */

/* Default values for missing or invalid fields */
#define UNKNOWN_VALUE "?"      /* Display placeholder for missing string fields */
#define ZERO_VALUE 0.0         /* Default for missing numeric fields */
#define MS_PER_SECOND 1000.0   /* Milliseconds to seconds conversion factor */
#define MS_TO_NANOSEC 1000000L /* Milliseconds to nanoseconds conversion */

/* Exit codes for different error conditions */
#define MCCS_ERROR_MEMORY 2 /* Memory allocation failure (malloc/realloc) */
#define MCCS_ERROR_IO 3     /* I/O operation failure (read/write/file ops) */
#define MCCS_ERROR_JSON 4   /* JSON parsing or validation failure */

/* Token tracking and session management constants */
#define BUF_SESSION_ID_SIZE 128          /* Buffer for session UUID strings */
#define BUF_TRANSCRIPT_PATH_SIZE 512     /* Path to session transcript JSONL files */
#define DEFAULT_TOKEN_LIMIT 200000       /* Claude's standard 200k context window limit */
#define TOKEN_SCALE_BILLION 1000000000.0 /* Scale factor for billion tokens (G suffix) */
#define TOKEN_SCALE_MILLION 1000000.0    /* Scale factor for million tokens (M suffix) */
#define TOKEN_SCALE_THOUSAND 1000.0      /* Scale factor for thousand tokens (K suffix) */
#define CACHE_MAX_AGE_S 60               /* Maximum cache age in seconds (safety limit) */
#define CACHE_DIR_MODE 0700              /* Directory permissions: rwx------ (user only) */

/* Display and UI constants */
#define PROGRESS_BAR_WIDTH 20   /* Width of progress bars in status display */
#define PROGRESS_BAR_FILLED "█" /* U+2588 Full block for filled progress segments */
#define PROGRESS_BAR_EMPTY "░"  /* U+2591 Light shade for empty progress segments */

/* Output format strings for status line display */

/**
 * Compact verbose format: cwd == project_dir
 * Includes "Model:", "Version:", etc. labels with colors
 */
#define FMT_STATUS_COMPACT_VERBOSE                 \
  "%s%sModel:%s %s%s%s (%s%s%s) %s|%s "            \
  "%sVersion:%s %s%s%s %s|%s "                     \
  "%sDirectory:%s %s%s%s %s|%s "                   \
  "%sCost:%s %s$%.4f%s %sTokens:%s %s%s%s %s|%s "  \
  "%sTotal:%s %s%.1fs%s %sAPI:%s %s%.1fs%s %s|%s " \
  "%sLines:%s %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n"

/**
 * Extended verbose format: cwd != project_dir
 * Shows both "Directory:" and "Project:" fields
 */
#define FMT_STATUS_EXTENDED_VERBOSE                \
  "%s%sModel:%s %s%s%s (%s%s%s) %s|%s "            \
  "%sVersion:%s %s%s%s %s|%s "                     \
  "%sDirectory:%s %s%s%s %s|%s "                   \
  "%sProject:%s %s%s%s %s|%s "                     \
  "%sCost:%s %s$%.4f%s %sTokens:%s %s%s%s %s|%s "  \
  "%sTotal:%s %s%.1fs%s %sAPI:%s %s%.1fs%s %s|%s " \
  "%sLines:%s %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n"

/**
 * Compact plain format: no field labels
 * Uses | separators between sections
 */
#define FMT_STATUS_COMPACT_PLAIN                                        \
  "%s%s%s%s (%s%s%s) | %s%s%s | %s%s%s | %s$%.4f%s %s%s%s | %s%.1fs%s " \
  "%s%.1fs%s | %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n"

/**
 * Extended plain format: no field labels, separate directory/project
 */
#define FMT_STATUS_EXTENDED_PLAIN                                      \
  "%s%s%s%s (%s%s%s) | %s%s%s | %s%s%s | %s%s%s | %s$%.4f%s %s%s%s | " \
  "%s%.1fs%s %s%.1fs%s | %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n"

/* JSON path arrays - NULL-terminated key sequences for navigation */
/* Example: PATH_MODEL_NAME navigates root["model"]["display_name"] */

static const char *const PATH_MODEL_NAME[] = {"model", "display_name", NULL};
static const char *const PATH_MODEL_ID[] = {"model", "id", NULL};
static const char *const PATH_CWD[] = {"cwd", NULL};
static const char *const PATH_PROJECT_DIR[] = {"workspace", "project_dir", NULL};
static const char *const PATH_VERSION[] = {"version", NULL};
static const char *const PATH_COST[] = {"cost", "total_cost_usd", NULL};
static const char *const PATH_DURATION[] = {"cost", "total_duration_ms", NULL};
static const char *const PATH_API_DURATION[] = {"cost", "total_api_duration_ms", NULL};
static const char *const PATH_LINES_ADDED[] = {"cost", "total_lines_added", NULL};
static const char *const PATH_LINES_REMOVED[] = {"cost", "total_lines_removed", NULL};
static const char *const PATH_EXCEEDS_200K[] = {"exceeds_200k_tokens", NULL};
static const char *const PATH_SESSION_ID[] = {"session_id", NULL};
static const char *const PATH_TRANSCRIPT_PATH[] = {"transcript_path", NULL};

#endif /* MCCS_CONSTANTS_H */
