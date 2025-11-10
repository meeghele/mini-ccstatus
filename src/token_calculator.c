// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "token_calculator.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "debug.h"
#include "lib/cjson/cJSON.h"
#include "safe_conv.h"

void init_token_counts(struct token_counts *tokens) {
  if (!tokens) {
    return;
  }
  tokens->input_tokens = 0;
  tokens->output_tokens = 0;
  tokens->cache_creation_tokens = 0;
  tokens->cache_read_tokens = 0;
  tokens->total_tokens = 0;
}

ResultU64 calculate_total_tokens(const struct token_counts *tokens) {
  if (!tokens) {
    return ERR(ResultU64, MCCS_ERR_INVALID_JSON);
  }

  ResultU64 sum_result = safe_add_uint64(tokens->input_tokens, tokens->output_tokens);
  if (IS_ERR(sum_result)) {
    DEBUG_LOG("WARNING: Token overflow in calculate_total_tokens (input/output)");
    return ERR(ResultU64, MCCS_ERR_OVERFLOW);
  }
  uint64_t sum = UNWRAP_OK(sum_result);

  sum_result = safe_add_uint64(sum, tokens->cache_creation_tokens);
  if (IS_ERR(sum_result)) {
    DEBUG_LOG("WARNING: Token overflow in calculate_total_tokens (cache_creation)");
    return ERR(ResultU64, MCCS_ERR_OVERFLOW);
  }
  sum = UNWRAP_OK(sum_result);

  sum_result = safe_add_uint64(sum, tokens->cache_read_tokens);
  if (IS_ERR(sum_result)) {
    DEBUG_LOG("WARNING: Token overflow in calculate_total_tokens (cache_read)");
    return ERR(ResultU64, MCCS_ERR_OVERFLOW);
  }

  return OK(ResultU64, UNWRAP_OK(sum_result));
}

void format_tokens(char *buf, size_t buf_size, uint64_t tokens) {
  if (!buf || buf_size == 0) {
    return;
  }

  if (tokens >= TOKEN_SCALE_BILLION) {
    snprintf(buf, buf_size, "%.1fG", (double)tokens / TOKEN_SCALE_BILLION);
  } else if (tokens >= TOKEN_SCALE_MILLION) {
    snprintf(buf, buf_size, "%.1fM", (double)tokens / TOKEN_SCALE_MILLION);
  } else if (tokens >= TOKEN_SCALE_THOUSAND) {
    snprintf(buf, buf_size, "%.1fK", (double)tokens / TOKEN_SCALE_THOUSAND);
  } else {
    snprintf(buf, buf_size, "%" PRIu64, tokens);
  }
}

uint32_t calculate_percentage(uint64_t tokens,
                              uint64_t limit,
                              bool clamp) {
  if (limit == 0) {
    return 0;
  }

  ResultU64 product_result = safe_mul_uint64(tokens, 100);
  if (IS_ERR(product_result)) {
    DEBUG_LOG("WARNING: Overflow in percentage calculation");
    return clamp ? 100 : UINT32_MAX;
  }

  uint64_t product = UNWRAP_OK(product_result);
  uint64_t pct = product / limit;
  if (clamp && pct > 100) {
    return 100;
  }

  return (uint32_t)(pct > UINT32_MAX ? UINT32_MAX : pct);
}

/**
 * Extract and accumulate token counts from a JSON usage object
 *
 * @param usage     JSON object containing token usage fields
 * @param tokens    Token counts structure to accumulate into
 * @return          ResultVoid - Ok if successful, Err on overflow or conversion error
 *
 * @note Supports both raw and aggregated naming conventions for cache tokens.
 * @error MCCS_ERR_INVALID_JSON if usage is NULL or not an object
 * @error MCCS_ERR_INVALID_CONVERSION if double to uint64 conversion fails
 * @error MCCS_ERR_OVERFLOW if token addition would overflow
 */
static ResultVoid extract_tokens_from_usage(const cJSON *usage, struct token_counts *tokens) {
  if (!usage || !cJSON_IsObject(usage)) {
    return ERR(ResultVoid, MCCS_ERR_INVALID_JSON);
  }

  const cJSON *input = cJSON_GetObjectItemCaseSensitive(usage, "input_tokens");
  const cJSON *output = cJSON_GetObjectItemCaseSensitive(usage, "output_tokens");

  const cJSON *cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_input_tokens");
  const cJSON *cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_input_tokens");

  if (!cache_creation) {
    cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_tokens");
  }
  if (!cache_read) {
    cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_tokens");
  }

  if (input && cJSON_IsNumber(input)) {
    ResultU64 temp_value_result = safe_double_to_uint64(input->valuedouble);
    if (IS_ERR(temp_value_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(temp_value_result));
    }
    uint64_t temp_value = UNWRAP_OK(temp_value_result);
    ResultU64 new_total_result = safe_add_uint64(tokens->input_tokens, temp_value);
    if (IS_ERR(new_total_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(new_total_result));
    }
    tokens->input_tokens = UNWRAP_OK(new_total_result);
  }
  if (output && cJSON_IsNumber(output)) {
    ResultU64 temp_value_result = safe_double_to_uint64(output->valuedouble);
    if (IS_ERR(temp_value_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(temp_value_result));
    }
    uint64_t temp_value = UNWRAP_OK(temp_value_result);
    ResultU64 new_total_result = safe_add_uint64(tokens->output_tokens, temp_value);
    if (IS_ERR(new_total_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(new_total_result));
    }
    tokens->output_tokens = UNWRAP_OK(new_total_result);
  }
  if (cache_creation && cJSON_IsNumber(cache_creation)) {
    ResultU64 temp_value_result = safe_double_to_uint64(cache_creation->valuedouble);
    if (IS_ERR(temp_value_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(temp_value_result));
    }
    uint64_t temp_value = UNWRAP_OK(temp_value_result);
    ResultU64 new_total_result = safe_add_uint64(tokens->cache_creation_tokens, temp_value);
    if (IS_ERR(new_total_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(new_total_result));
    }
    tokens->cache_creation_tokens = UNWRAP_OK(new_total_result);
  }
  if (cache_read && cJSON_IsNumber(cache_read)) {
    ResultU64 temp_value_result = safe_double_to_uint64(cache_read->valuedouble);
    if (IS_ERR(temp_value_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(temp_value_result));
    }
    uint64_t temp_value = UNWRAP_OK(temp_value_result);
    ResultU64 new_total_result = safe_add_uint64(tokens->cache_read_tokens, temp_value);
    if (IS_ERR(new_total_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(new_total_result));
    }
    tokens->cache_read_tokens = UNWRAP_OK(new_total_result);
  }

  return OK(ResultVoid, 0);
}

ResultTokenCounts parse_session_tokens(const char *session_path) {
  DEBUG_LOG("Parsing session tokens from: %s", session_path);
  FILE *fp = fopen(session_path, "r");
  if (!fp) {
    DEBUG_LOG("Failed to open transcript file: %s", session_path);
    return ERR(ResultTokenCounts, MCCS_ERR_FILE_NOT_FOUND);
  }

  struct token_counts tokens;
  init_token_counts(&tokens);

  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  size_t line_count = 0;

  while ((len = getline(&line, &cap, fp)) != -1) {
    line_count++;
    if (len <= 1) {
      continue;
    }

    cJSON *entry = cJSON_ParseWithLength(line, (size_t)len);
    if (!entry) {
      continue;
    }

    const cJSON *message = cJSON_GetObjectItemCaseSensitive(entry, "message");
    if (message && cJSON_IsObject(message)) {
      const cJSON *usage = cJSON_GetObjectItemCaseSensitive(message, "usage");
      ResultVoid extract_result = extract_tokens_from_usage(usage, &tokens);
      if (IS_ERR(extract_result)) {
        cJSON_Delete(entry);
        free(line);
        fclose(fp);
        return ERR(ResultTokenCounts, UNWRAP_ERR(extract_result));
      }
    }

    cJSON_Delete(entry);
  }

  free(line);
  fclose(fp);

  ResultU64 total_result = calculate_total_tokens(&tokens);
  if (IS_ERR(total_result)) {
    return ERR(ResultTokenCounts, UNWRAP_ERR(total_result));
  }
  tokens.total_tokens = UNWRAP_OK(total_result);
  DEBUG_LOG("Parsed %zu lines, total tokens: %lu", line_count, tokens.total_tokens);
  return OK(ResultTokenCounts, tokens);
}

ResultU64 count_context_tokens(const char *transcript_path) {
  DEBUG_LOG("Counting context tokens from: %s", transcript_path);
  FILE *fp = fopen(transcript_path, "r");
  if (!fp) {
    DEBUG_LOG("Failed to open transcript file for context count");
    return ERR(ResultU64, MCCS_ERR_FILE_NOT_FOUND);
  }

  uint64_t context_tokens = 0;
  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  char *last_assistant_line = NULL;
  size_t last_assistant_cap = 0;

  while ((len = getline(&line, &cap, fp)) != -1) {
    if (len <= 1) {
      continue;
    }

    cJSON *entry = cJSON_ParseWithLength(line, (size_t)len);
    if (entry) {
      const cJSON *message = cJSON_GetObjectItemCaseSensitive(entry, "message");
      if (message && cJSON_IsObject(message)) {
        const cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
        if (role && cJSON_IsString(role)) {
          const char *role_str = cJSON_GetStringValue(role);
          if (role_str && strcmp(role_str, "assistant") == 0) {
            if ((size_t)len > last_assistant_cap) {
              char *new_buf = realloc(last_assistant_line, (size_t)len + 1);
              if (new_buf) {
                last_assistant_line = new_buf;
                last_assistant_cap = (size_t)len + 1;
              }
            }
            if (last_assistant_line && (size_t)len <= last_assistant_cap) {
              memcpy(last_assistant_line, line, (size_t)len);
              last_assistant_line[len] = '\0';
            }
          }
        }
      }
      cJSON_Delete(entry);
    }
  }

  free(line);
  fclose(fp);

  if (last_assistant_line) {
    cJSON *entry = cJSON_Parse(last_assistant_line);
    if (entry) {
      const cJSON *message = cJSON_GetObjectItemCaseSensitive(entry, "message");
      if (message && cJSON_IsObject(message)) {
        const cJSON *usage = cJSON_GetObjectItemCaseSensitive(message, "usage");
        if (usage && cJSON_IsObject(usage)) {
          const cJSON *input = cJSON_GetObjectItemCaseSensitive(usage, "input_tokens");
          const cJSON *cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_input_tokens");
          const cJSON *cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_input_tokens");

          if (!cache_creation) {
            cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_tokens");
          }
          if (!cache_read) {
            cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_tokens");
          }

          if (input && cJSON_IsNumber(input)) {
            ResultU64 temp_value_result = safe_double_to_uint64(input->valuedouble);
            if (IS_OK(temp_value_result)) {
              uint64_t temp_value = UNWRAP_OK(temp_value_result);
              ResultU64 new_total_result = safe_add_uint64(context_tokens, temp_value);
              if (IS_OK(new_total_result)) {
                context_tokens = UNWRAP_OK(new_total_result);
              }
            }
          }
          if (cache_creation && cJSON_IsNumber(cache_creation)) {
            ResultU64 temp_value_result = safe_double_to_uint64(cache_creation->valuedouble);
            if (IS_OK(temp_value_result)) {
              uint64_t temp_value = UNWRAP_OK(temp_value_result);
              ResultU64 new_total_result = safe_add_uint64(context_tokens, temp_value);
              if (IS_OK(new_total_result)) {
                context_tokens = UNWRAP_OK(new_total_result);
              }
            }
          }
          if (cache_read && cJSON_IsNumber(cache_read)) {
            ResultU64 temp_value_result = safe_double_to_uint64(cache_read->valuedouble);
            if (IS_OK(temp_value_result)) {
              uint64_t temp_value = UNWRAP_OK(temp_value_result);
              ResultU64 new_total_result = safe_add_uint64(context_tokens, temp_value);
              if (IS_OK(new_total_result)) {
                context_tokens = UNWRAP_OK(new_total_result);
              }
            }
          }
        }
      }
      cJSON_Delete(entry);
    }
    free(last_assistant_line);
    DEBUG_LOG("Context tokens from last assistant message: %lu", context_tokens);
    return OK(ResultU64, context_tokens);
  }

  DEBUG_LOG("No assistant message found in transcript");
  return OK(ResultU64, 0);
}

ResultVoid parse_tokens_single_pass(const char *transcript_path,
                                    struct token_counts *session_tokens,
                                    uint64_t *context_tokens) {
  DEBUG_LOG("Single-pass parsing tokens from: %s", transcript_path);

  if (!session_tokens && !context_tokens) {
    DEBUG_LOG("No output requested");
    return OK(ResultVoid, 0);
  }

  FILE *fp = fopen(transcript_path, "r");
  if (!fp) {
    DEBUG_LOG("Failed to open transcript file: %s", transcript_path);
    return ERR(ResultVoid, MCCS_ERR_FILE_NOT_FOUND);
  }

  if (session_tokens) {
    init_token_counts(session_tokens);
  }
  if (context_tokens) {
    *context_tokens = 0;
  }

  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  size_t line_count = 0;

  uint64_t last_assistant_input = 0;
  bool found_assistant = false;

  while ((len = getline(&line, &cap, fp)) != -1) {
    line_count++;
    if (len <= 1) {
      continue;
    }

    cJSON *entry = cJSON_ParseWithLength(line, (size_t)len);
    if (!entry) {
      continue;
    }

    const cJSON *message = cJSON_GetObjectItemCaseSensitive(entry, "message");
    if (message && cJSON_IsObject(message)) {
      const cJSON *usage = cJSON_GetObjectItemCaseSensitive(message, "usage");

      if (session_tokens && usage) {
        ResultVoid extract_result = extract_tokens_from_usage(usage, session_tokens);
        if (IS_ERR(extract_result)) {
          cJSON_Delete(entry);
          free(line);
          fclose(fp);
          return ERR(ResultVoid, UNWRAP_ERR(extract_result));
        }
      }

      if (context_tokens) {
        const cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
        if (role && cJSON_IsString(role)) {
          const char *role_str = cJSON_GetStringValue(role);
          if (role_str && strcmp(role_str, "assistant") == 0 && usage) {
            uint64_t total_context = 0;

            const cJSON *input = cJSON_GetObjectItemCaseSensitive(usage, "input_tokens");
            const cJSON *cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_input_tokens");
            const cJSON *cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_input_tokens");

            if (!cache_creation) {
              cache_creation = cJSON_GetObjectItemCaseSensitive(usage, "cache_creation_tokens");
            }
            if (!cache_read) {
              cache_read = cJSON_GetObjectItemCaseSensitive(usage, "cache_read_tokens");
            }

            if (input && cJSON_IsNumber(input)) {
              ResultU64 temp_value_result = safe_double_to_uint64(input->valuedouble);
              if (IS_OK(temp_value_result)) {
                uint64_t temp_value = UNWRAP_OK(temp_value_result);
                ResultU64 total_context_result = safe_add_uint64(total_context, temp_value);
                if (IS_OK(total_context_result)) {
                  total_context = UNWRAP_OK(total_context_result);
                }
              }
            }
            if (cache_creation && cJSON_IsNumber(cache_creation)) {
              ResultU64 temp_value_result = safe_double_to_uint64(cache_creation->valuedouble);
              if (IS_OK(temp_value_result)) {
                uint64_t temp_value = UNWRAP_OK(temp_value_result);
                ResultU64 total_context_result = safe_add_uint64(total_context, temp_value);
                if (IS_OK(total_context_result)) {
                  total_context = UNWRAP_OK(total_context_result);
                }
              }
            }
            if (cache_read && cJSON_IsNumber(cache_read)) {
              ResultU64 temp_value_result = safe_double_to_uint64(cache_read->valuedouble);
              if (IS_OK(temp_value_result)) {
                uint64_t temp_value = UNWRAP_OK(temp_value_result);
                ResultU64 total_context_result = safe_add_uint64(total_context, temp_value);
                if (IS_OK(total_context_result)) {
                  total_context = UNWRAP_OK(total_context_result);
                }
              }
            }

            if (total_context > 0) {
              last_assistant_input = total_context;
              found_assistant = true;
              DEBUG_LOG("Found assistant message with %lu total context tokens", last_assistant_input);
            }
          }
        }
      }
    }

    cJSON_Delete(entry);
  }

  free(line);
  fclose(fp);

  if (session_tokens) {
    ResultU64 total_result = calculate_total_tokens(session_tokens);
    if (IS_ERR(total_result)) {
      return ERR(ResultVoid, UNWRAP_ERR(total_result));
    }
    session_tokens->total_tokens = UNWRAP_OK(total_result);
    DEBUG_LOG("Parsed %zu lines, total session tokens: %lu", line_count, session_tokens->total_tokens);
  }

  if (context_tokens && found_assistant) {
    *context_tokens = last_assistant_input;
    DEBUG_LOG("Context tokens from last assistant: %lu", *context_tokens);
  } else if (context_tokens) {
    DEBUG_LOG("No assistant message found for context");
  }

  return OK(ResultVoid, 0);
}
