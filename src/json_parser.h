// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file json_parser.h
 * @brief JSON parsing utilities for mini-ccstatus
 *
 * Provides functions for parsing JSON documents, navigating object hierarchies,
 * and extracting typed values. Handles graceful degradation for missing fields.
 */

#ifndef MCCS_JSON_PARSER_H
#define MCCS_JSON_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lib/cjson/cJSON.h"
#include "result.h"
#include "token_calculator.h"
#include "types_struct.h"

// Result types for JSON parsing
DEFINE_RESULT(ResultJson, cJSON *, enum MccsError);

/**
 * Parse a JSON document from a buffer
 *
 * @param buffer     JSON string buffer to parse
 * @param length     Length of the buffer
 * @return           Result<cJSON*> - Ok with parsed JSON or Err with error code
 *
 * @note Caller must call cJSON_Delete() on the returned object if Result is Ok.
 * @error MCCS_ERR_INVALID_JSON on parse error, MCCS_ERR_OUT_OF_MEMORY on OOM
 */
ResultJson parse_json_document(const char *restrict buffer, size_t length);

/**
 * Navigate a JSON object tree following a path of keys
 *
 * @param root    Root JSON object to search
 * @param keys    NULL-terminated array of key names to traverse
 * @return        JSON node at the specified path, or NULL if not found
 *
 * @note Uses case-sensitive key matching.
 */
const cJSON *find_path(const cJSON *root, const char *const *keys);

/**
 * Load a string field from JSON into a buffer
 *
 * @param root       Root JSON object to search
 * @param path       NULL-terminated array of keys to traverse
 * @param buffer     Destination buffer for the string value
 * @param capacity   Size of the destination buffer
 * @param out        Output: pointer set to buffer on success
 * @return           ResultVoid - Ok if field found and loaded, Err with error code otherwise
 *
 * @error MCCS_ERR_INVALID_JSON if parameters are NULL or field not found/wrong type
 * @error MCCS_ERR_BUFFER_TOO_SMALL should never occur (truncation is allowed)
 */
ResultVoid load_string_field(const cJSON *restrict root,
                             const char *const *restrict path,
                             char *restrict buffer,
                             size_t capacity,
                             const char **restrict out);

/**
 * Load a double-precision floating point field from JSON
 *
 * @param root    Root JSON object to search
 * @param path    NULL-terminated array of keys to traverse
 * @param out     Output: value set on success
 * @return        ResultVoid - Ok if field found and is a number, Err with error code otherwise
 *
 * @error MCCS_ERR_INVALID_JSON if parameters are NULL or field not found/not a number
 */
ResultVoid load_double_field(const cJSON *root,
                             const char *const *path,
                             double *out);

/**
 * Load an unsigned 32-bit integer field from JSON
 *
 * @param root    Root JSON object to search
 * @param path    NULL-terminated array of keys to traverse
 * @param out     Output: value set on success
 * @return        ResultVoid - Ok if field found and within uint32_t range, Err with error code otherwise
 *
 * @error MCCS_ERR_INVALID_JSON if parameters are NULL or field not found/not a number
 * @error MCCS_ERR_INVALID_CONVERSION if value is out of uint32_t range
 */
ResultVoid load_uint32_field(const cJSON *root,
                             const char *const *path,
                             uint32_t *out);

/**
 * Load a boolean field from JSON
 *
 * @param root    Root JSON object to search
 * @param path    NULL-terminated array of keys to traverse
 * @param out     Output: value set on success
 * @return        ResultVoid - Ok if field found and is boolean, Err with error code otherwise
 *
 * @error MCCS_ERR_INVALID_JSON if parameters are NULL or field not found/not a boolean
 */
ResultVoid load_bool_field(const cJSON *root,
                           const char *const *path,
                           bool *out);

/**
 * Initialize a status structure with default values
 *
 * @param status    Status structure to initialize
 */
void init_mccs_status(struct mccs_status *status);

/**
 * Load all status fields from JSON into mccs_status structure
 *
 * @param root      Root JSON object from Claude Code
 * @param status    Output structure (must be initialized first)
 */
void load_mccs_status(const cJSON *root, struct mccs_status *status);

/**
 * Load session_id and transcript_path from JSON
 *
 * @param root    Root JSON object
 * @param paths   Output structure for paths
 * @return        ResultVoid - Ok if at least one path was loaded, Err if both failed
 *
 * @error MCCS_ERR_INVALID_JSON if parameters are NULL or both paths missing
 */
ResultVoid load_mccs_paths(const cJSON *root, struct mccs_paths *paths);

#endif /* MCCS_JSON_PARSER_H */
