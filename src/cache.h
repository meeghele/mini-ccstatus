// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file cache.h
 * @brief Session-based file cache for token statistics
 *
 * Implements a persistent cache to avoid re-parsing large transcript files.
 * Cache files are stored per-session in /tmp/mini-ccstatus/<uid>/<session_id>.cache
 * with file locking to prevent race conditions.
 */

#ifndef MCCS_CACHE_H
#define MCCS_CACHE_H

#include <stdbool.h>
#include <stddef.h>

#include "result.h"
#include "types_struct.h"

#define CACHE_MAGIC 0xCCCC0002

// Result types for cache operations
DEFINE_RESULT(ResultTokenCache, struct token_cache, enum MccsError);
DEFINE_RESULT(ResultVoidCache, int, enum MccsError);

/**
 * Get the filesystem path for a session's cache file
 *
 * @param session_id    Unique session identifier (NULL for default cache)
 * @return              Static buffer containing cache file path
 *
 * @note Creates cache directory /tmp/mini-ccstatus/<uid>/ if needed
 * @note Returns pointer to static buffer - not thread safe
 */
const char *get_cache_path(const char *session_id);

/**
 * Load cache from disk for a specific session
 *
 * @param session_id    Session identifier to load cache for
 * @return              Result<TokenCache> - Ok with cache or Err with error code
 *
 * @note Uses shared file lock (LOCK_SH) to prevent reading during writes
 * @note Validates magic number and cache age before returning success
 * @error MCCS_ERR_FILE_NOT_FOUND if cache doesn't exist or can't be opened
 * @error MCCS_ERR_INVALID_FORMAT if cache magic number is wrong
 */
ResultTokenCache load_cache(const char *session_id);

/**
 * Save cache to disk for a specific session
 *
 * @param cache         Cache structure to persist
 * @param session_id    Session identifier for cache file
 * @return              ResultVoid - Ok(0) on success or Err with error code
 *
 * @note Uses exclusive file lock (LOCK_EX) to prevent concurrent writes
 * @error MCCS_ERR_FILE_NOT_FOUND if cache file cannot be created
 * @error MCCS_ERR_IO_ERROR if write fails
 */
ResultVoidCache save_cache(const struct token_cache *cache, const char *session_id);

/**
 * Check if cache is valid for the current session
 *
 * @param cache         Cache to validate
 * @param session_id    Current session identifier
 * @param project_dir   Current project directory
 * @return              true if cache matches session and is not expired
 *
 * @note Checks magic number, session_id, project_dir, and age
 */
bool is_cache_valid(const struct token_cache *cache,
                    const char *session_id,
                    const char *project_dir);

/**
 * Determine if cache needs to be refreshed
 *
 * @param cache            Current cache state
 * @param session_id       Current session identifier
 * @param project_dir      Current project directory
 * @param transcript_path  Path to transcript file (for size check)
 * @return                 true if cache should be regenerated
 *
 * @note Returns true if session changed, cache invalid, or transcript file size changed
 */
bool should_refresh_cache(const struct token_cache *cache,
                          const char *session_id,
                          const char *project_dir,
                          const char *transcript_path);

#endif /* MCCS_CACHE_H */
