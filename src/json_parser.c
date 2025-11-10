// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "json_parser.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "colors.h"
#include "constants.h"
#include "debug.h"
#include "safe_conv.h"

/**
 * Replace all whitespace control characters with spaces
 *
 * @param str    String to sanitize (modified in-place)
 * @param len    Length of string to process
 *
 * @note Replaces newlines (\n), carriage returns (\r), and tabs (\t) with spaces.
 *       This ensures single-line output for status display.
 */
static void sanitize_whitespace(char *str,
                                size_t len) {
  if (!str || len == 0) {
    return;
  }
  for (size_t i = 0; i < len; ++i) {
    if (str[i] == '\0') {
      break;
    }
    if (str[i] == '\n' || str[i] == '\r' || str[i] == '\t') {
      str[i] = ' ';
    }
  }
}

const cJSON *find_path(const cJSON *root, const char *const *keys) {
  if (!root || !keys) {
    return NULL;
  }

  const cJSON *node = root;
  for (int level = 0; keys[level] != NULL; ++level) {
    if (!cJSON_IsObject(node)) {
      return NULL;
    }
    node = cJSON_GetObjectItemCaseSensitive(node, keys[level]);
    if (!node) {
      return NULL;
    }
  }
  return node;
}

ResultJson parse_json_document(const char *restrict buffer, size_t length) {
  if (!buffer) {
    return ERR(ResultJson, MCCS_ERR_INVALID_JSON);
  }

  DEBUG_LOG("Parsing JSON document of length %zu", length);
  cJSON *root = cJSON_ParseWithLength(buffer, length);
  if (!root) {
    if (cJSON_GetErrorPtr() == NULL) {
      DEBUG_LOG("JSON parse failed: out of memory");
      fprintf(MCCS_STDERR, "error: out of memory\n");
      return ERR(ResultJson, MCCS_ERR_OUT_OF_MEMORY);
    } else {
      DEBUG_LOG("JSON parse failed: syntax error near position %ld", cJSON_GetErrorPtr() - buffer);
      return ERR(ResultJson, MCCS_ERR_INVALID_JSON);
    }
  }

  DEBUG_LOG("JSON parsed successfully");
  return OK(ResultJson, root);
}

void init_mccs_status(struct mccs_status *status) {
  if (!status) {
    return;
  }
  status->string_refs.model_name = UNKNOWN_VALUE;
  status->string_refs.model_id = UNKNOWN_VALUE;
  status->string_refs.cwd = UNKNOWN_VALUE;
  status->string_refs.project_dir = UNKNOWN_VALUE;
  status->string_refs.version = UNKNOWN_VALUE;

  status->counters.cost_usd = NAN;
  status->counters.duration_ms = 0;
  status->counters.api_ms = 0;
  status->counters.lines_added = 0;
  status->counters.lines_removed = 0;
  status->counters.exceeds_200k_tokens = false;

  status->buffers.buf_model_name[0] = '\0';
  status->buffers.buf_model_id[0] = '\0';
  status->buffers.buf_cwd[0] = '\0';
  status->buffers.buf_project[0] = '\0';
  status->buffers.buf_version[0] = '\0';
}

ResultVoid load_string_field(const cJSON *restrict root,
                             const char *const *restrict path,
                             char *restrict buffer,
                             size_t capacity,
                             const char **restrict out) {
  if (!root || !path || !buffer || !out || capacity == 0) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }

  const cJSON *node = find_path(root, path);
  if (!cJSON_IsString(node)) {
    return ERR(ResultVoid, MCCS_ERR_MISSING_FIELD);
  }

  const char *value = cJSON_GetStringValue(node);
  if (!value) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }

  size_t len = strlen(value);
  if (len >= capacity) {
    len = capacity - 1;
  }
  memcpy(buffer, value, len);
  buffer[len] = '\0';
  sanitize_whitespace(buffer, len);
  *out = buffer;
  return OK(ResultVoid, 0);
}

ResultVoid load_double_field(const cJSON *root,
                             const char *const *path,
                             double *out) {
  if (!root || !path || !out) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsNumber(node)) {
    return ERR(ResultVoid, MCCS_ERR_MISSING_FIELD);
  }

  *out = node->valuedouble;
  return OK(ResultVoid, 0);
}

ResultVoid load_uint32_field(const cJSON *root,
                             const char *const *path,
                             uint32_t *out) {
  if (!root || !path || !out) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsNumber(node)) {
    return ERR(ResultVoid, MCCS_ERR_MISSING_FIELD);
  }

  double value = node->valuedouble;

  ResultU32 result = safe_double_to_uint32(value);
  if (IS_ERR(result)) {
    return ERR(ResultVoid, UNWRAP_ERR(result));
  }
  *out = UNWRAP_OK(result);
  return OK(ResultVoid, 0);
}

ResultVoid load_bool_field(const cJSON *root,
                           const char *const *path,
                           bool *out) {
  if (!root || !path || !out) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsBool(node)) {
    return ERR(ResultVoid, MCCS_ERR_MISSING_FIELD);
  }

  *out = cJSON_IsTrue(node);
  return OK(ResultVoid, 0);
}

void load_mccs_status(const cJSON *root,
                      struct mccs_status *status) {
  if (!root || !status) {
    return;
  }
  DEBUG_LOG("Loading status fields from JSON");
  struct mccs_buffers *buffers = &status->buffers;
  struct mccs_string_refs *refs = &status->string_refs;
  struct mccs_counters *counters = &status->counters;

  (void)load_string_field(root,
                          PATH_MODEL_NAME,
                          buffers->buf_model_name,
                          sizeof(buffers->buf_model_name),
                          &refs->model_name);
  (void)load_string_field(root,
                          PATH_MODEL_ID,
                          buffers->buf_model_id,
                          sizeof(buffers->buf_model_id),
                          &refs->model_id);
  (void)load_string_field(root,
                          PATH_CWD,
                          buffers->buf_cwd,
                          sizeof(buffers->buf_cwd),
                          &refs->cwd);
  (void)load_string_field(root,
                          PATH_PROJECT_DIR,
                          buffers->buf_project,
                          sizeof(buffers->buf_project),
                          &refs->project_dir);
  (void)load_string_field(root,
                          PATH_VERSION,
                          buffers->buf_version,
                          sizeof(buffers->buf_version),
                          &refs->version);

  (void)load_double_field(root,
                          PATH_COST,
                          &counters->cost_usd);
  (void)load_uint32_field(root,
                          PATH_DURATION,
                          &counters->duration_ms);
  (void)load_uint32_field(root,
                          PATH_API_DURATION,
                          &counters->api_ms);
  (void)load_uint32_field(root,
                          PATH_LINES_ADDED,
                          &counters->lines_added);
  (void)load_uint32_field(root,
                          PATH_LINES_REMOVED,
                          &counters->lines_removed);

  (void)load_bool_field(root, PATH_EXCEEDS_200K,
                        &counters->exceeds_200k_tokens);

  DEBUG_LOG("Loaded: model=%s, version=%s, cwd=%s",
            refs->model_name, refs->version, refs->cwd);
}

ResultVoid load_mccs_paths(const cJSON *root,
                           struct mccs_paths *paths) {
  if (!root || !paths) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }
  const char *session_id_ptr = NULL;
  const char *transcript_ptr = NULL;

  ResultVoid session_id_result = load_string_field(root,
                                                   PATH_SESSION_ID,
                                                   paths->session_id,
                                                   sizeof(paths->session_id),
                                                   &session_id_ptr);

  ResultVoid transcript_result = load_string_field(root,
                                                   PATH_TRANSCRIPT_PATH,
                                                   paths->transcript_path,
                                                   sizeof(paths->transcript_path),
                                                   &transcript_ptr);

  // Return Ok if at least one path was loaded successfully
  if (IS_OK(session_id_result) || IS_OK(transcript_result)) {
    return OK(ResultVoid, 0);
  }

  return ERR(ResultVoid, MCCS_ERR_MISSING_FIELD);
}
