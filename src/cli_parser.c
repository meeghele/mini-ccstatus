// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "cli_parser.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "result.h"

void mccs_print_usage(const char *prog_name) {
  printf("Usage: %s [OPTIONS]\n\n", prog_name);
  printf("Claude Code status line generator - reads JSON from stdin and outputs formatted status.\n\n");
  printf("Options:\n");
  printf("  -h, --help                      Show this help message and exit\n");
  printf("  -d, --token-breakdown           Show detailed token breakdown\n");
  printf("  -c, --context-tokens            Show context window percentage\n");
  printf("  -t, --session-tokens            Show session total tokens\n");
  printf("  -e, --cache-efficiency          Show cache efficiency ratio\n");
  printf("  -p, --api-time-ratio            Show API time vs total time ratio\n");
  printf("  -l, --lines-ratio               Show lines added vs removed ratio\n");
  printf("  -i, --input-output-ratio        Show input vs output tokens ratio\n");
  printf("  -w, --cache-write-read-ratio    Show cache write vs read tokens ratio\n");
  printf("  -C, --clamping                  Clamp percentages to 100%%%% max\n");
  printf("  -a, --all                       Enable all token features\n");
  printf("      --no-color                  Disable ANSI color output\n");
  printf("  -v, --verbose                   Show field labels in status line\n");
  printf("  -H, --hide-breakdown            Hide token breakdown line\n");
  printf("  -s, --simple                    Show simplified status line (Model/Version/Directory only)\n\n");
  printf("Environment Variables:\n");
  printf("  NO_COLOR                 If set, disables ANSI color output\n\n");
  printf("Examples:\n");
  printf("  echo '{...}' | %s\n", prog_name);
  printf("  %s --all < status.json\n", prog_name);
  printf("  %s --verbose --context-tokens < status.json\n", prog_name);
}

void mccs_init_cli_options(struct cli_options *opts) {
  if (!opts) {
    return;
  }
  opts->show_token_breakdown = false;
  opts->show_context_tokens = false;
  opts->show_session_tokens = false;
  opts->show_cache_efficiency = false;
  opts->show_api_time_ratio = false;
  opts->show_lines_ratio = false;
  opts->show_input_output_ratio = false;
  opts->show_cache_write_read_ratio = false;
  opts->clamp_percentages = false;
  opts->show_all = false;
  opts->no_color = false;
  opts->verbose = false;
  opts->hide_token_breakdown = false;
  opts->simple_status_line = false;
}

ResultVoid mccs_parse_cli_args(int argc,
                               char *argv[],
                               struct cli_options *opts) {
  if (!opts || !argv) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }

  mccs_init_cli_options(opts);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      mccs_print_usage(argv[0]);
      exit(0);
    } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--token-breakdown") == 0) {
      opts->show_token_breakdown = true;
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--context-tokens") == 0) {
      opts->show_context_tokens = true;
    } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--session-tokens") == 0) {
      opts->show_session_tokens = true;
    } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--cache-efficiency") == 0) {
      opts->show_cache_efficiency = true;
    } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--api-time-ratio") == 0) {
      opts->show_api_time_ratio = true;
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lines-ratio") == 0) {
      opts->show_lines_ratio = true;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input-output-ratio") == 0) {
      opts->show_input_output_ratio = true;
    } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--cache-write-read-ratio") == 0) {
      opts->show_cache_write_read_ratio = true;
    } else if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--clamping") == 0) {
      opts->clamp_percentages = true;
    } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
      opts->show_all = true;
      opts->show_token_breakdown = true;
      opts->show_context_tokens = true;
      opts->show_session_tokens = true;
      opts->show_cache_efficiency = true;
      opts->show_api_time_ratio = true;
      opts->show_lines_ratio = true;
      opts->show_input_output_ratio = true;
      opts->show_cache_write_read_ratio = true;
    } else if (strcmp(argv[i], "--no-color") == 0) {
      opts->no_color = true;
    } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      opts->verbose = true;
    } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--hide-breakdown") == 0) {
      opts->hide_token_breakdown = true;
    } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--simple") == 0) {
      opts->simple_status_line = true;
    }
  }

  return OK(ResultVoid, 0);
}
