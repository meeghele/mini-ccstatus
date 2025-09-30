// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/cjson/cJSON.h"

#define BUF_MODEL_NAME_SIZE 64
#define BUF_MODEL_ID_SIZE 128
#define BUF_PATH_SIZE 256
#define BUF_VERSION_SIZE 32
#define MAX_INPUT_LINE_SIZE (1024 * 1024) // 1MB limit

#define UNKNOWN_VALUE "?"
#define ZERO_VALUE 0.0
#define MS_PER_SECOND 1000.0

#define ERROR_MEMORY 2
#define ERROR_IO 3
#define ERROR_JSON 4

#define COLOR_NONE ""
#define COLOR_RESET "\033[0m"
#define COLOR_LABEL "\033[38;5;15m"
#define COLOR_MODEL_NAME "\033[38;5;105m"
#define COLOR_MODEL_ID "\033[38;5;147m"
#define COLOR_VERSION "\033[38;5;214m"
#define COLOR_DIR "\033[38;5;39m"
#define COLOR_COST "\033[38;5;226m"
#define COLOR_TIME_TOTAL "\033[38;5;141m"
#define COLOR_TIME_API "\033[38;5;183m"
#define COLOR_LINES_ADDED "\033[38;5;34m"
#define COLOR_LINES_REMOVED "\033[38;5;160m"
#define COLOR_BADGE_UNDER "\033[38;5;34m"
#define COLOR_BADGE_OVER "\033[38;5;160m"

static const char *const FMT_STATUS_COMPACT_VERBOSE =
    "%s%sModel:%s %s%s%s (%s%s%s) %s|%s "
    "%sVersion:%s %s%s%s %s|%s "
    "%sDirectory:%s %s%s%s %s|%s "
    "%sTokens:%s %s%s%s %sCost:%s %s$%.4f%s %s|%s "
    "%sTotal:%s %s%.1fs%s %sAPI:%s %s%.1fs%s %s|%s "
    "%sLines:%s %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n";

static const char *const FMT_STATUS_EXTENDED_VERBOSE =
    "%s%sModel:%s %s%s%s (%s%s%s) %s|%s "
    "%sVersion:%s %s%s%s %s|%s "
    "%sDirectory:%s %s%s%s %s|%s "
    "%sProject:%s %s%s%s %s|%s "
    "%sTokens:%s %s%s%s %sCost:%s %s$%.4f%s %s|%s "
    "%sTotal:%s %s%.1fs%s %sAPI:%s %s%.1fs%s %s|%s "
    "%sLines:%s %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n";

static const char *const FMT_STATUS_COMPACT_PLAIN =
    "%s%s%s%s (%s%s%s) | %s%s%s | %s%s%s | %s%s%s %s$%.4f%s | %s%.1fs%s "
    "%s%.1fs%s | %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n";

static const char *const FMT_STATUS_EXTENDED_PLAIN =
    "%s%s%s%s (%s%s%s) | %s%s%s | %s%s%s | %s%s%s | %s%s%s %s$%.4f%s | "
    "%s%.1fs%s %s%.1fs%s | %s+%" PRIu32 "%s/%s-%" PRIu32 "%s\n";

struct mccs_buffers {
  char buf_model_name[BUF_MODEL_NAME_SIZE];
  char buf_model_id[BUF_MODEL_ID_SIZE];
  char buf_cwd[BUF_PATH_SIZE];
  char buf_project[BUF_PATH_SIZE];
  char buf_version[BUF_VERSION_SIZE];
};

struct mccs_string_refs {
  const char *model_name;
  const char *model_id;
  const char *cwd;
  const char *project_dir;
  const char *version;
};

struct mccs_counters {
  double cost_usd;
  uint32_t duration_ms;
  uint32_t api_ms;
  uint32_t lines_added;
  uint32_t lines_removed;
  bool exceeds_200k_tokens;
};

struct mccs_status {
  struct mccs_buffers buffers;
  struct mccs_string_refs string_refs;
  struct mccs_counters counters;
};

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

static void sanitize_whitespace(char *str, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (str[i] == '\n' || str[i] == '\r' || str[i] == '\t') {
      str[i] = ' ';
    }
  }
}

static const char *extract_basename_inplace(char *path) {
  if (!path || !path[0]) {
    return UNKNOWN_VALUE;
  }

  char *end = path + strlen(path) - 1;
  while (end > path && *end == '/') {
    *end = '\0';
    --end;
  }

  if (end == path && *end == '/') {
    return "/";
  }

  char *slash = end;
  while (slash > path && *(slash - 1) != '/') {
    --slash;
  }

  return slash;
}

static char *read_line_from_stdin(size_t *out_len, size_t *out_bytes_read, int *exit_code) {
  char *line = NULL;
  size_t cap = 0;
  ssize_t raw_len = getline(&line, &cap, stdin);

  if (raw_len == -1) {
    const char *c_reset = COLOR_RESET;
    if (ferror(stdin)) {
      printf("%serror: read failed\n", c_reset);
      *exit_code = ERROR_IO;
    } else {
      *exit_code = 0;
    }
    free(line);
    return NULL;
  }

  // Enforce maximum input size limit
  if ((size_t)raw_len > MAX_INPUT_LINE_SIZE) {
    const char *c_reset = COLOR_RESET;
    printf("%serror: input exceeds maximum size limit\n", c_reset);
    free(line);
    *exit_code = ERROR_IO;
    return NULL;
  }

  size_t bytes_read = (size_t)raw_len;
  ssize_t len = raw_len;
  if (len > 0 && line[len - 1] == '\n') {
    line[len - 1] = '\0';
    len--;
  }

  *out_len = (size_t)len;
  if (out_bytes_read) {
    *out_bytes_read = bytes_read;
  }
  *exit_code = 0;
  return line;
}

static const cJSON *find_path(const cJSON *root, const char *const *keys) {
  if (!root) {
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

static cJSON *parse_json_document(const char *buffer, size_t length, int *exit_code) {
  if (!buffer) {
    *exit_code = ERROR_JSON;
    return NULL;
  }

  cJSON *root = cJSON_ParseWithLength(buffer, length);
  if (!root) {
    const char *c_reset = COLOR_RESET;
    if (cJSON_GetErrorPtr() == NULL) {
      printf("%serror: out of memory\n", c_reset);
      *exit_code = ERROR_MEMORY;
    } else {
      *exit_code = ERROR_JSON;
    }
    return NULL;
  }

  return root;
}

static void init_mccs_status(struct mccs_status *status) {
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

static bool load_string_field(const cJSON *root, const char *const *path, char *buffer, size_t capacity, const char **out) {
  if (capacity == 0) {
    return false;
  }

  const cJSON *node = find_path(root, path);
  if (!cJSON_IsString(node)) {
    return false;
  }

  const char *value = cJSON_GetStringValue(node);
  if (!value) {
    return false;
  }

  size_t len = strlen(value);
  if (len >= capacity) {
    len = capacity - 1;
  }
  memcpy(buffer, value, len);
  buffer[len] = '\0';
  sanitize_whitespace(buffer, len);
  *out = buffer;
  return true;
}

static bool load_double_field(const cJSON *root, const char *const *path, double *out) {
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsNumber(node)) {
    return false;
  }

  *out = node->valuedouble;
  return true;
}

static bool load_uint32_field(const cJSON *root, const char *const *path, uint32_t *out) {
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsNumber(node)) {
    return false;
  }

  double value = node->valuedouble;

  // Check for underflow, overflow, NaN, and infinity
  if (value < 0.0 || value > (double)UINT32_MAX || !isfinite(value)) {
    return false;
  }

  *out = (uint32_t)value;
  return true;
}

static bool load_bool_field(const cJSON *root, const char *const *path, bool *out) {
  const cJSON *node = find_path(root, path);
  if (!cJSON_IsBool(node)) {
    return false;
  }

  *out = cJSON_IsTrue(node);
  return true;
}

static void load_mccs_status(const cJSON *root, struct mccs_status *status) {
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
}

static const char *get_color(bool use_color, const char *color) {
  return use_color ? color : COLOR_NONE;
}

static void print_mccs_status_line(bool use_color, const struct mccs_status *status) {
  const struct mccs_string_refs *refs = &status->string_refs;
  const struct mccs_counters *counters = &status->counters;
  const struct mccs_buffers *buffers = &status->buffers;

  char cwd_copy[BUF_PATH_SIZE];
  char proj_copy[BUF_PATH_SIZE];

  double cost = isnan(counters->cost_usd) ? ZERO_VALUE : counters->cost_usd;
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
    cwd_display = extract_basename_inplace(cwd_copy);
  }

  if (refs->project_dir == buffers->buf_project) {
    (void)snprintf(proj_copy,
                   sizeof(proj_copy),
                   "%s",
                   buffers->buf_project);
    proj_display = extract_basename_inplace(proj_copy);
  }

  const char *c_reset = get_color(use_color, COLOR_RESET);
  const char *c_label = get_color(use_color, COLOR_LABEL);
  const char *c_model_name = get_color(use_color, COLOR_MODEL_NAME);
  const char *c_model_id = get_color(use_color, COLOR_MODEL_ID);
  const char *c_version = get_color(use_color, COLOR_VERSION);
  const char *c_dir = get_color(use_color, COLOR_DIR);
  const char *c_cost = get_color(use_color, COLOR_COST);
  const char *c_time_total = get_color(use_color, COLOR_TIME_TOTAL);
  const char *c_time_api = get_color(use_color, COLOR_TIME_API);
  const char *c_lines_added = get_color(use_color, COLOR_LINES_ADDED);
  const char *c_lines_removed = get_color(use_color, COLOR_LINES_REMOVED);

  bool use_verbose = getenv("VERBOSE") != NULL;

  printf("%s", c_reset);

  const char *badge_text = counters->exceeds_200k_tokens ? ">200k" : "<200k";
  const char *c_badge = counters->exceeds_200k_tokens
                            ? get_color(use_color, COLOR_BADGE_OVER)
                            : get_color(use_color, COLOR_BADGE_UNDER);

  if (strcmp(cwd_display, proj_display) == 0) {
    if (use_verbose) {
      printf(FMT_STATUS_COMPACT_VERBOSE,
             c_reset,                                 // initial reset
             c_label, c_reset,                        // "Model:" label
             c_model_name, refs->model_name, c_reset, // model name value
             c_model_id, refs->model_id, c_reset,     // model id value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Version:" label
             c_version, refs->version, c_reset,       // version value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Directory:" label
             c_dir, cwd_display, c_reset,             // directory value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Tokens:" label
             c_badge, badge_text, c_reset,            // badge value
             c_label, c_reset,                        // "Cost:" label
             c_cost, cost, c_reset,                   // cost value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Total:" label
             c_time_total, dur_s, c_reset,            // total time value
             c_label, c_reset,                        // "API:" label
             c_time_api, api_s, c_reset,              // API time value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Lines:" label
             c_lines_added, added, c_reset,           // lines added value
             c_lines_removed, removed, c_reset);      // lines removed value
    } else {
      printf(FMT_STATUS_COMPACT_PLAIN,
             c_reset,                                 // initial reset
             c_model_name, refs->model_name, c_reset, // model name value
             c_model_id, refs->model_id, c_reset,     // model id value
             c_version, refs->version, c_reset,       // version value
             c_dir, cwd_display, c_reset,             // directory value
             c_badge, badge_text, c_reset,            // badge value
             c_cost, cost, c_reset,                   // cost value
             c_time_total, dur_s, c_reset,            // total time value
             c_time_api, api_s, c_reset,              // API time value
             c_lines_added, added, c_reset,           // lines added value
             c_lines_removed, removed, c_reset);      // lines removed value
    }
  } else {
    if (use_verbose) {
      printf(FMT_STATUS_EXTENDED_VERBOSE,
             c_reset,                                 // initial reset
             c_label, c_reset,                        // "Model:" label
             c_model_name, refs->model_name, c_reset, // model name value
             c_model_id, refs->model_id, c_reset,     // model id value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Version:" label
             c_version, refs->version, c_reset,       // version value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Directory:" label
             c_dir, cwd_display, c_reset,             // directory value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Project:" label
             c_dir, proj_display, c_reset,            // project value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Tokens:" label
             c_badge, badge_text, c_reset,            // badge value
             c_label, c_reset,                        // "Cost:" label
             c_cost, cost, c_reset,                   // cost value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Total:" label
             c_time_total, dur_s, c_reset,            // total time value
             c_label, c_reset,                        // "API:" label
             c_time_api, api_s, c_reset,              // API time value
             c_label, c_reset,                        // separator "|"
             c_label, c_reset,                        // "Lines:" label
             c_lines_added, added, c_reset,           // lines added value
             c_lines_removed, removed, c_reset);      // lines removed value
    } else {
      printf(FMT_STATUS_EXTENDED_PLAIN,
             c_reset,                                 // initial reset
             c_model_name, refs->model_name, c_reset, // model name value
             c_model_id, refs->model_id, c_reset,     // model id value
             c_version, refs->version, c_reset,       // version value
             c_dir, cwd_display, c_reset,             // directory value
             c_dir, proj_display, c_reset,            // project value
             c_badge, badge_text, c_reset,            // badge value
             c_cost, cost, c_reset,                   // cost value
             c_time_total, dur_s, c_reset,            // total time value
             c_time_api, api_s, c_reset,              // API time value
             c_lines_added, added, c_reset,           // lines added value
             c_lines_removed, removed, c_reset);      // lines removed value
    }
  }
}

static void process_complete_json(bool use_color, const char *buf, size_t len, int *exit_code) {
  cJSON *root = parse_json_document(buf, len, exit_code);
  if (!root) {
    if (*exit_code != 0 && *exit_code != ERROR_JSON) {
      return;
    }

    if (*exit_code == ERROR_JSON) {
      const char *c_reset = COLOR_RESET;
      printf("%serror: invalid JSON\n", c_reset);
    }

    return;
  }

  struct mccs_status status;
  init_mccs_status(&status);
  load_mccs_status(root, &status);
  print_mccs_status_line(use_color, &status);
  cJSON_Delete(root);
}

static int process_json_stream(bool use_color) {
  int exit_code = 0;
  size_t line_len = 0;
  char *line = read_line_from_stdin(&line_len, NULL, &exit_code);

  if (!line) {
    return exit_code;
  }

  process_complete_json(use_color, line, line_len, &exit_code);
  free(line);

  return exit_code;
}

int main(void) {
  bool use_color = getenv("NO_COLOR") == NULL;
  return process_json_stream(use_color);
}
