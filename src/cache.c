// Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
// Licensed under the MIT License. See LICENSE file for details.

#include "cache.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "debug.h"
#include "safe_conv.h"
#include "token_calculator.h"

#define CACHE_HASH_FNV_OFFSET 1469598103934665603ULL
#define CACHE_HASH_FNV_PRIME 1099511628211ULL
#define CACHE_HASH_OUTPUT_SIZE 17 // 16 hex chars + null terminator

#define CACHE_DIR_PATH "/tmp/mini-ccstatus"
#define CACHE_FALLBACK_PATH "/tmp/mini-ccstatus-fallback.cache"
#define CACHE_LOCK_TIMEOUT_MS 2000
#define CACHE_LOCK_INTERVAL_MS 50

/**
 * Get file size safely
 *
 * @param path    Path to file
 * @return        File size in bytes, or 0 if file doesn't exist or error
 */
static size_t get_file_size(const char *path) {
  if (!path || !*path) {
    return 0;
  }

  struct stat st;
  if (stat(path, &st) != 0) {
    return 0;
  }

  ResultSize size_result = safe_off_to_size(st.st_size);
  if (IS_ERR(size_result)) {
    DEBUG_LOG("WARNING: File size conversion failed for %s", path);
    return 0;
  }

  return UNWRAP_OK(size_result);
}

/**
 * Attempt to acquire an advisory lock with a bounded wait
 *
 * @param fd           File descriptor to lock
 * @param operation    flock operation (LOCK_SH or LOCK_EX)
 * @param timeout_ms   Maximum time to wait before giving up
 * @return             ResultVoid - Ok if lock acquired, Err on timeout or error
 *
 * @error MCCS_ERR_IO_ERROR if lock acquisition fails or times out
 */
static ResultVoid acquire_lock_with_timeout(int fd,
                                            int operation,
                                            unsigned int timeout_ms) {
  unsigned int waited_ms = 0;

  while (true) {
    if (flock(fd, operation | LOCK_NB) == 0) {
      return OK(ResultVoid, 0);
    }

    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      DEBUG_LOG("Failed to acquire %s lock: %s",
                (operation & LOCK_EX) ? "exclusive" : "shared",
                strerror(errno));
      return ERR(ResultVoid, MCCS_ERR_IO_ERROR);
    }

    if (waited_ms >= timeout_ms) {
      DEBUG_LOG("Timed out acquiring %s lock after %u ms",
                (operation & LOCK_EX) ? "exclusive" : "shared",
                waited_ms);
      errno = EWOULDBLOCK;
      return ERR(ResultVoid, MCCS_ERR_IO_ERROR);
    }

    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = (long)CACHE_LOCK_INTERVAL_MS * MS_TO_NANOSEC,
    };
    nanosleep(&ts, NULL);
    waited_ms += CACHE_LOCK_INTERVAL_MS;
  }
}

/**
 * Hash a session identifier to a filesystem-safe string
 *
 * @param session_id   Original session identifier
 * @param out          Output buffer for hexadecimal hash string
 * @param out_size     Size of the output buffer
 */
static void hash_session_id(const char *session_id,
                            char *out,
                            size_t out_size) {
  if (!out || out_size < CACHE_HASH_OUTPUT_SIZE) {
    return;
  }

  if (!session_id || *session_id == '\0') {
    snprintf(out, out_size, "default");
    return;
  }

  uint64_t hash = CACHE_HASH_FNV_OFFSET;

  for (const unsigned char *p = (const unsigned char *)session_id; *p; ++p) {
    hash ^= (uint64_t)(*p);
    hash *= CACHE_HASH_FNV_PRIME;
  }

  snprintf(out, out_size, "%016llx", (unsigned long long)hash);
}

const char *get_cache_path(const char *session_id) {
  static char path[BUF_PATH_SIZE];

  char cache_dir[BUF_PATH_SIZE];
  snprintf(cache_dir, sizeof(cache_dir), "%s/%u", CACHE_DIR_PATH, (unsigned int)getuid());

  struct stat st = {0};
  if (stat(CACHE_DIR_PATH, &st) == -1) {
    mkdir(CACHE_DIR_PATH, CACHE_DIR_MODE);
  }
  if (stat(cache_dir, &st) == -1) {
    mkdir(cache_dir, CACHE_DIR_MODE);
  }

  int ret;
  if (!session_id || !*session_id) {
    // Fallback to user-wide cache if no session_id available
    ret = snprintf(path, sizeof(path), "%s/default.cache", cache_dir);
  } else {
    char hashed_session[CACHE_HASH_OUTPUT_SIZE] = {0};
    hash_session_id(session_id, hashed_session, sizeof(hashed_session));
    ret = snprintf(path, sizeof(path), "%s/%s.cache", cache_dir, hashed_session);
  }

  if (ret < 0 || (size_t)ret >= sizeof(path)) {
    DEBUG_LOG("Cache path truncated or encoding error");
    snprintf(path, sizeof(path), "%s", CACHE_FALLBACK_PATH);
  }

  DEBUG_LOG("Cache path: %s", path);
  return path;
}

ResultTokenCache load_cache(const char *session_id) {
  const char *path = get_cache_path(session_id);
  DEBUG_LOG("Loading cache from: %s", path);

  FILE *f = fopen(path, "rb");
  if (!f) {
    DEBUG_LOG("Cache file not found or cannot be opened");
    return ERR(ResultTokenCache, MCCS_ERR_FILE_NOT_FOUND);
  }

  int fd = fileno(f);
  // Use blocking lock with alarm timeout for safety
  // This ensures atomic reads while preventing indefinite hangs
  ResultVoid lock_result = acquire_lock_with_timeout(fd, LOCK_SH, CACHE_LOCK_TIMEOUT_MS);
  if (IS_ERR(lock_result)) {
    fclose(f);
    return ERR(ResultTokenCache, UNWRAP_ERR(lock_result));
  }

  struct token_cache cache;
  size_t n = fread(&cache, sizeof(cache), 1, f);

  flock(fd, LOCK_UN);
  fclose(f);

  if (n != 1) {
    DEBUG_LOG("Cache read failed or incomplete");
    return ERR(ResultTokenCache, MCCS_ERR_IO_ERROR);
  }

  if (cache.magic != CACHE_MAGIC) {
    DEBUG_LOG("Cache magic number mismatch: expected 0x%X, got 0x%X",
              CACHE_MAGIC, cache.magic);
    return ERR(ResultTokenCache, MCCS_ERR_INVALID_FORMAT);
  }

  int64_t now = (int64_t)time(NULL);
  int64_t age = now - cache.last_update_time;
  if (age > CACHE_MAX_AGE_S) {
    DEBUG_LOG("Cache expired: age=%ld seconds, max=%d", (long)age, CACHE_MAX_AGE_S);
    return ERR(ResultTokenCache, MCCS_ERR_INVALID_FORMAT);
  }

  DEBUG_LOG("Cache loaded successfully (age=%ld seconds)", (long)age);
  return OK(ResultTokenCache, cache);
}

ResultVoidCache save_cache(const struct token_cache *cache,
                           const char *session_id) {
  const char *path = get_cache_path(session_id);
  DEBUG_LOG("Saving cache to: %s", path);

  FILE *f = fopen(path, "wb");
  if (!f) {
    DEBUG_LOG("Failed to open cache file for writing");
    return ERR(ResultVoidCache, MCCS_ERR_FILE_NOT_FOUND);
  }

  int fd = fileno(f);
  // Use blocking lock with alarm timeout for safety
  // This ensures atomic writes while preventing indefinite hangs
  ResultVoid lock_result = acquire_lock_with_timeout(fd, LOCK_EX, CACHE_LOCK_TIMEOUT_MS);
  if (IS_ERR(lock_result)) {
    fclose(f);
    return ERR(ResultVoidCache, UNWRAP_ERR(lock_result));
  }

  size_t written = fwrite(cache, sizeof(*cache), 1, f);

  flock(fd, LOCK_UN);
  fclose(f);

  if (written != 1) {
    DEBUG_LOG("Cache write failed");
    return ERR(ResultVoidCache, MCCS_ERR_IO_ERROR);
  }

  DEBUG_LOG("Cache saved successfully");
  return OK(ResultVoidCache, 0);
}

bool is_cache_valid(const struct token_cache *cache,
                    const char *session_id,
                    const char *project_dir) {
  if (!cache || cache->magic != CACHE_MAGIC) {
    DEBUG_LOG("Cache invalid: bad magic number");
    return false;
  }

  if (session_id && strcmp(cache->session_id, session_id) != 0) {
    DEBUG_LOG("Cache invalid: session ID mismatch");
    return false;
  }

  if (project_dir && strcmp(cache->project_dir, project_dir) != 0) {
    DEBUG_LOG("Cache invalid: project directory mismatch");
    return false;
  }

  int64_t now = (int64_t)time(NULL);
  int64_t age = now - cache->last_update_time;
  if (age > CACHE_MAX_AGE_S) {
    DEBUG_LOG("Cache invalid: expired (age=%ld)", (long)age);
    return false;
  }

  DEBUG_LOG("Cache is valid");
  return true;
}

bool should_refresh_cache(const struct token_cache *cache,
                          const char *session_id,
                          const char *project_dir,
                          const char *transcript_path) {
  if (!is_cache_valid(cache, session_id, project_dir)) {
    DEBUG_LOG("Cache refresh needed: invalid cache");
    return true;
  }

  size_t current_size = get_file_size(transcript_path);
  if (current_size != cache->transcript_file_size) {
    DEBUG_LOG("Cache refresh needed: file size changed (cached=%zu, current=%zu)",
              cache->transcript_file_size, current_size);
    return true;
  }

  DEBUG_LOG("Cache is fresh, no refresh needed (file size unchanged)");
  return false;
}
