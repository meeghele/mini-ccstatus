// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file main.c
 * @brief Main entry point for mini-ccstatus
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "lib/cjson/cJSON.h"
#include "src/cache.h"
#include "src/cli_parser.h"
#include "src/colors.h"
#include "src/constants.h"
#include "src/debug.h"
#include "src/display.h"
#include "src/json_parser.h"
#include "src/safe_conv.h"
#include "src/token_calculator.h"
#include "src/types_struct.h"

struct stdin_line {
  char *line;        // Must be freed by caller
  size_t len;
  size_t bytes_read;
};

DEFINE_RESULT(ResultStdinLine, struct stdin_line, enum MccsError);

/**
 * Read a single line from standard input
 *
 * @return ResultStdinLine - Ok with line data on success, Err on EOF or error
 *
 * @note Caller must free() the line field in the result.
 *       Enforces MAX_INPUT_LINE_SIZE limit.
 *       Strips trailing newline if present.
 * @error MCCS_ERR_IO_ERROR on read failure or input too large
 * @error MCCS_ERR_INVALID_CONVERSION on internal size conversion error
 */
static ResultStdinLine mccs_read_stdin_line(void) {
  char *line = NULL;
  size_t cap = 0;
  ssize_t raw_len = getline(&line, &cap, stdin);

  if (raw_len == -1) {
    if (ferror(stdin)) {
      fprintf(MCCS_STDERR, "error: read failed\n");
      free(line);
      return ERR(ResultStdinLine, MCCS_ERR_IO_ERROR);
    } else {
      free(line);
      return ERR(ResultStdinLine, MCCS_ERR_IO_ERROR);
    }
  }

  ResultSize bytes_read_result = safe_ssize_to_size(raw_len);
  if (IS_ERR(bytes_read_result)) {
    fprintf(MCCS_STDERR, "error: invalid line length\n");
    free(line);
    return ERR(ResultStdinLine, MCCS_ERR_INVALID_CONVERSION);
  }
  size_t bytes_read = UNWRAP_OK(bytes_read_result);

  if (bytes_read > MAX_INPUT_LINE_SIZE) {
    fprintf(MCCS_STDERR, "error: input exceeds maximum size limit\n");
    free(line);
    return ERR(ResultStdinLine, MCCS_ERR_BUFFER_TOO_SMALL);
  }

  size_t len = bytes_read;
  if (len > 0 && line[len - 1] == '\n') {
    line[len - 1] = '\0';
    len--;
  }

  struct stdin_line result = {
      .line = line,
      .len = len,
      .bytes_read = bytes_read};
  return OK(ResultStdinLine, result);
}

/**
 * Process a complete JSON input and output the status line
 *
 * @param use_color    Whether to use ANSI color codes
 * @param use_verbose  Whether to show field labels
 * @param opts         CLI options for display formatting
 * @param buffer       JSON string buffer
 * @param length       Length of buffer
 * @return             ResultVoid - Ok(0) on success or Err with error code
 */
static ResultVoid mccs_process_json(bool use_color,
                                    bool use_verbose,
                                    const struct cli_options *opts,
                                    const char *buffer,
                                    size_t length) {
  ResultJson root_result = parse_json_document(buffer, length);
  if (IS_ERR(root_result)) {
    return ERR(ResultVoid, UNWRAP_ERR(root_result));
  }

  cJSON *root = UNWRAP_OK(root_result);

  struct mccs_status status;
  init_mccs_status(&status);
  load_mccs_status(root, &status);
  print_mccs_status_line(use_color, use_verbose, &status, opts->simple_status_line);

  struct mccs_paths paths = {0};
  ResultVoid paths_result = load_mccs_paths(root, &paths);
  bool has_paths = IS_OK(paths_result);

  bool needs_session_tokens = opts->show_token_breakdown ||
                              opts->show_session_tokens ||
                              opts->show_cache_efficiency ||
                              opts->show_input_output_ratio ||
                              opts->show_cache_write_read_ratio ||
                              opts->show_all;

  bool needs_context_tokens = opts->show_context_tokens ||
                              opts->show_all;

  bool needs_token_parsing = needs_session_tokens ||
                             needs_context_tokens;

  struct token_counts session_tokens;
  init_token_counts(&session_tokens);
  bool session_tokens_parsed = false;
  uint64_t context_tokens = 0;
  bool context_tokens_parsed = false;

  if (has_paths && paths.transcript_path[0] != '\0' && needs_token_parsing) {
    ResultTokenCache cache_result = load_cache(paths.session_id);
    bool cache_loaded = IS_OK(cache_result);

    struct token_cache cache = {0};
    if (cache_loaded) {
      cache = UNWRAP_OK(cache_result);
    }

    bool should_refresh = should_refresh_cache(&cache,
                                               paths.session_id,
                                               status.buffers.buf_project,
                                               paths.transcript_path);

    bool needs_refresh = !cache_loaded || should_refresh;

    if (cache_loaded && !needs_refresh) {
      DEBUG_LOG("Using cached token data");
      session_tokens = cache.session_tokens;
      session_tokens_parsed = true;
      context_tokens = cache.context_tokens.total_tokens;
      context_tokens_parsed = (context_tokens > 0);
    } else {
      DEBUG_LOG("Cache miss or expired, parsing token data");

      if (needs_session_tokens && needs_context_tokens) {
        ResultVoid result = parse_tokens_single_pass(paths.transcript_path, &session_tokens, &context_tokens);
        if (IS_OK(result)) {
          session_tokens_parsed = true;
          context_tokens_parsed = (context_tokens > 0);
        }
      } else {
        if (needs_session_tokens) {
          ResultTokenCounts result = parse_session_tokens(paths.transcript_path);
          if (IS_OK(result)) {
            session_tokens = UNWRAP_OK(result);
            session_tokens_parsed = true;
          }
        }

        if (needs_context_tokens) {
          ResultU64 result = count_context_tokens(paths.transcript_path);
          if (IS_OK(result)) {
            context_tokens = UNWRAP_OK(result);
            context_tokens_parsed = (context_tokens > 0);
          }
        }
      }

      cache.magic = CACHE_MAGIC;
      cache.last_update_time = (int64_t)time(NULL);
      strncpy(cache.session_id, paths.session_id, BUF_SESSION_ID_SIZE - 1);
      cache.session_id[BUF_SESSION_ID_SIZE - 1] = '\0';
      strncpy(cache.project_dir, status.buffers.buf_project, BUF_PATH_SIZE - 1);
      cache.project_dir[BUF_PATH_SIZE - 1] = '\0';

      if (session_tokens_parsed) {
        cache.session_tokens = session_tokens;
      }
      if (context_tokens_parsed) {
        init_token_counts(&cache.context_tokens);
        cache.context_tokens.total_tokens = context_tokens;
      }

      struct stat st;
      if (stat(paths.transcript_path, &st) == 0) {
        ResultSize size_result = safe_off_to_size(st.st_size);
        cache.transcript_file_size = IS_OK(size_result) ? UNWRAP_OK(size_result) : 0;
      } else {
        cache.transcript_file_size = 0;
      }

      (void)save_cache(&cache, paths.session_id);
    }
  }

  if ((opts->show_context_tokens || opts->show_all) && context_tokens_parsed) {
    print_context_percentage(use_color, use_verbose, context_tokens, opts->clamp_percentages);
  }

  if ((opts->show_session_tokens || opts->show_all) && session_tokens_parsed) {
    print_session_total(use_color, use_verbose, session_tokens.total_tokens, opts->clamp_percentages);
  }

  if ((opts->show_cache_efficiency || opts->show_all) && session_tokens_parsed) {
    print_cache_efficiency(use_color, use_verbose, &session_tokens);
  }

  if (opts->show_api_time_ratio || opts->show_all) {
    print_api_time_ratio(use_color, use_verbose, status.counters.api_ms, status.counters.duration_ms);
  }

  if (opts->show_lines_ratio || opts->show_all) {
    print_lines_ratio(use_color, use_verbose, status.counters.lines_added, status.counters.lines_removed);
  }

  if ((opts->show_input_output_ratio || opts->show_all) && session_tokens_parsed) {
    print_input_output_ratio(use_color, use_verbose, &session_tokens);
  }

  if ((opts->show_cache_write_read_ratio || opts->show_all) && session_tokens_parsed) {
    print_cache_write_read_ratio(use_color, use_verbose, &session_tokens);
  }

  if ((opts->show_token_breakdown || opts->show_all) && session_tokens_parsed && !opts->hide_token_breakdown) {
    print_token_breakdown(use_color, use_verbose, &session_tokens);
  }

  cJSON_Delete(root);
  return OK(ResultVoid, 0);
}

/**
 * Process JSON input from stdin in streaming mode
 *
 * @param use_color    Whether to use ANSI color codes
 * @param use_verbose  Whether to show field labels
 * @param opts         CLI options for display formatting
 * @return             Exit code (0 on success)
 */
static int mccs_process_stream(bool use_color,
                               bool use_verbose,
                               const struct cli_options *opts) {
  DEBUG_LOG("Reading JSON from stdin");
  ResultStdinLine stdin_result = mccs_read_stdin_line();

  if (IS_ERR(stdin_result)) {
    enum MccsError err = UNWRAP_ERR(stdin_result);
    if (err == MCCS_ERR_BUFFER_TOO_SMALL) {
      // Input exceeded size limit
      return MCCS_ERROR_IO;
    }
    if (err == MCCS_ERR_INVALID_CONVERSION) {
      // Internal error
      return MCCS_ERROR_IO;
    }
    // MCCS_ERR_IO_ERROR - could be EOF or read error
    // EOF is the common case and is not an error, so return 0
    DEBUG_LOG("No input received (EOF or read error)");
    return 0;
  }

  struct stdin_line stdin_data = UNWRAP_OK(stdin_result);
  DEBUG_LOG("Processing JSON line of length %zu", stdin_data.len);
  ResultVoid result = mccs_process_json(use_color, use_verbose, opts, stdin_data.line, stdin_data.len);
  free(stdin_data.line);

  if (IS_ERR(result)) {
    enum MccsError err = UNWRAP_ERR(result);
    if (err == MCCS_ERR_OUT_OF_MEMORY) {
      return MCCS_ERROR_MEMORY;
    } else if (err == MCCS_ERR_INVALID_JSON) {
      const struct color_theme *theme = get_theme(use_color);
      if (use_color) {
        fprintf(MCCS_STDERR, "%serror: invalid JSON\n", theme->reset);
      } else {
        fprintf(MCCS_STDERR, "error: invalid JSON\n");
      }
      return MCCS_ERROR_JSON;
    }
    return 1; // Generic error
  }

  return 0;
}

/**
 * Program entry point
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 on success, MCCS_ERROR_MEMORY/MCCS_ERROR_IO/MCCS_ERROR_JSON on failure)
 *
 * Environment variables:
 * - NO_COLOR: If set (any value), disables ANSI color output (overridden by --no-color)
 */
int main(int argc,
         char *argv[]) {
  struct cli_options opts;

  ResultVoid parse_result = mccs_parse_cli_args(argc, argv, &opts);
  if (IS_ERR(parse_result)) {
    return 1;
  }

  bool use_color = true;
  if (opts.no_color || getenv("NO_COLOR") != NULL) {
    use_color = false;
  }

  bool use_verbose = opts.verbose;

  DEBUG_LOG("mini-ccstatus starting (color=%s, verbose=%s, breakdown=%s, context=%s, session=%s, all=%s)",
            use_color ? ON : OFF,
            use_verbose ? ON : OFF,
            opts.show_token_breakdown ? ON : OFF,
            opts.show_context_tokens ? ON : OFF,
            opts.show_session_tokens ? ON : OFF,
            opts.show_all ? ON : OFF);

  return mccs_process_stream(use_color, use_verbose, &opts);
}
