// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file cli_parser.h
 * @brief Command-line argument parsing utilities
 *
 * Functions for parsing and validating command-line options.
 */

#ifndef MCCS_CLI_PARSER_H
#define MCCS_CLI_PARSER_H

#include <stdbool.h>

#include "result.h"
#include "token_calculator.h"
#include "types_struct.h"

/**
 * Initialize CLI options with defaults
 *
 * @param opts    Options structure to initialize
 */
void mccs_init_cli_options(struct cli_options *restrict opts);

/**
 * Parse command-line arguments
 *
 * @param argc    Argument count
 * @param argv    Argument vector
 * @param opts    Output: parsed options
 * @return        ResultVoid - Ok to continue execution, Err to exit
 *
 * @error MCCS_ERR_INVALID_JSON if arguments are NULL (treated as fatal error)
 * @note When help is requested, function prints usage and returns Ok(0)
 */
ResultVoid mccs_parse_cli_args(int argc,
                                char *argv[],
                                struct cli_options *restrict opts);

/**
 * Print usage information to stdout
 *
 * @param prog_name    Name of the program (argv[0])
 */
void mccs_print_usage(const char *prog_name);

#endif /* MCCS_CLI_PARSER_H */
