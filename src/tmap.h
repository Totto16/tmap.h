/*
 * tmap.h
 *
 * This file is part of the z-libs collection: https://github.com/z-libs
 * Licensed under the MIT License.
 *
 * based on
 * https://github.com/z-libs/zmap.h/blob/743b276cfccf65b8f4a07aa1175c12fc5359b525/zmap.h
 *
 * modified to suit my needs
 *
 * Modifications by: Totto16
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Return Codes.
typedef enum : bool {
  TmapResultErr = false,
  TmapResultOk = true,
} TmapResult;

#define TMAP_WOULD_OVERWRITE ((void *)(1))
#define TMAP_NO_ELEMENT_HERE ((void *)(2))

typedef enum : uint8_t {
  TmapInsertResultOk = 0,
  TmapInsertResultErr = 1,
  TmapInsertResultWouldOverwrite = 2
} TmapInsertResult;

// Memory Macros.
// If the user hasn't defined their own allocator, use the standard one.
#ifndef Z_MALLOC
#define Z_MALLOC(sz) malloc(sz)
#define Z_CALLOC(n, sz) calloc(n, sz)
#define Z_REALLOC(p, sz) realloc(p, sz)
#define Z_FREE(p) free(p)
#endif

// Compiler Extensions (Optional).
// We check for GCC/Clang features to enable RAII-style cleanup.
// Define Z_NO_EXTENSIONS to disable this manually.
#if !defined(Z_NO_EXTENSIONS) && (defined(__GNUC__) || defined(__clang__))
#define Z_HAS_CLEANUP 1
#define Z_CLEANUP(func) __attribute__((cleanup(func)))
#else
#define Z_HAS_CLEANUP 0
#define Z_CLEANUP(func)
#endif

#ifndef Z_MAP_MALLOC
#define Z_MAP_MALLOC(sz) Z_MALLOC(sz)
#endif
#ifndef Z_MAP_CALLOC
#define Z_MAP_CALLOC(n, sz) Z_CALLOC(n, sz)
#endif
#ifndef Z_MAP_FREE
#define Z_MAP_FREE(p) Z_FREE(p)
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) ||              \
    defined(__cplusplus)
#define STATIC_ASSERT(check, message) static_assert(check, message)
#elif __STDC_VERSION__ < 201112L
// empty, as not supported
#define STATIC_ASSERT(check, message)
#else
#define STATIC_ASSERT(check, message) _Static_assert(check, message)
#endif

// maybe some visibility things later, but I just removed the
// TMAP_FUN_ATTRIBUTES
#define TMAP_FUN_ATTRIBUTES

typedef size_t TmapHashType;

TMAP_FUN_ATTRIBUTES TmapHashType tmap_default_hash(const void *key, size_t len);

TMAP_FUN_ATTRIBUTES TmapHashType tmap_stbds_hash_string(const char *str);

TMAP_FUN_ATTRIBUTES TmapHashType tmap_stbds_hash_bytes(const void *key,
                                                       size_t len);

typedef enum { TMAP_EMPTY = 0, TMAP_OCCUPIED, TMAP_DELETED } tmap_state;

#define TMAP_HASH_SCALAR(k) tmap_default_hash(&(k), sizeof(k))
#define TMAP_HASH_STR(k) tmap_stbds_hash_string(k)
#define TMAP_HASH_BYTES(start, len) tmap_stbds_hash_bytes((start), len)

#define TMAP_HASH_FUNC_NAME(KeyName) tmap_hash_type_impl_##KeyName

#define TMAP_COMPARE_FUNC_NAME(KeyName) tmap_compare_type_impl_##KeyName

#define TMAP_HASH_FUNC_SIG(KeyT, KeyName)                                      \
  TMAP_FUN_ATTRIBUTES TmapHashType TMAP_HASH_FUNC_NAME(KeyName)(const KeyT key)

#define TMAP_COMPARE_FUNC_SIG(KeyT, KeyName)                                   \
  TMAP_FUN_ATTRIBUTES int TMAP_COMPARE_FUNC_NAME(KeyName)(const KeyT key1,     \
                                                          const KeyT key2)

#define TMAP_TYPENAME_ENTRY(TypeName) tmap_entry_##TypeName

#define TMAP_TYPENAME_BUCKET(TypeName) tmap_bucket_##TypeName

#define TMAP_TYPENAME_MAP(TypeName) tmap_##TypeName

#define TMAP_DEFINE_MAP_TYPE(KeyT, KeyName, ValT, Name)                        \
                                                                               \
  typedef struct {                                                             \
    KeyT key;                                                                  \
    ValT value;                                                                \
  } TMAP_TYPENAME_ENTRY(Name);                                                 \
                                                                               \
  typedef struct {                                                             \
    TMAP_TYPENAME_ENTRY(Name) entry;                                           \
    tmap_state state;                                                          \
  } TMAP_TYPENAME_BUCKET(Name);                                                \
                                                                               \
  typedef struct {                                                             \
    TMAP_TYPENAME_BUCKET(Name) * buckets;                                      \
    size_t capacity;                                                           \
    size_t count;                                                              \
    size_t occupied;                                                           \
  } TMAP_TYPENAME_MAP(Name);                                                   \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_free_##Name(TMAP_TYPENAME_MAP(Name) * m);      \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_TYPENAME_MAP(Name)                    \
      tmap_init_##Name(void);                                                  \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TmapResult tmap_resize_##Name(             \
      TMAP_TYPENAME_MAP(Name) * m, size_t new_cap);                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] ValT *tmap_insert_slot_##Name(             \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, bool allow_overwrite);            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TmapInsertResult tmap_insert_##Name(       \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, ValT const val,                   \
      bool allow_overwrite);                                                   \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] inline ValT *tmap_put_slot_##Name(         \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key) {                                 \
    return tmap_insert_slot_##Name(m, key, true);                              \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] inline TmapResult tmap_put_##Name(         \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, ValT const val) {                 \
    const TmapInsertResult result = tmap_insert_##Name(m, key, val, true);     \
    return result == TmapInsertResultOk ? TmapResultOk : TmapResultErr;        \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] ValT *tmap_get_mut_##Name(                 \
      TMAP_TYPENAME_MAP(Name) * m, const KeyT key);                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] ValT const *tmap_get_##Name(               \
      const TMAP_TYPENAME_MAP(Name) * m, const KeyT key);                      \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] const TMAP_TYPENAME_ENTRY(Name) *          \
      tmap_get_entry_##Name(const TMAP_TYPENAME_MAP(Name) * m,                 \
                            const KeyT key);                                   \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] const TMAP_TYPENAME_ENTRY(Name) *          \
      tmap_get_entry_at_##Name(const TMAP_TYPENAME_MAP(Name) * m,              \
                               size_t index);                                  \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_remove_##Name(TMAP_TYPENAME_MAP(Name) * m,     \
                                              const KeyT key);                 \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_clear_##Name(TMAP_TYPENAME_MAP(Name) * m);     \
                                                                               \
  TMAP_HASH_FUNC_SIG(KeyT, KeyName);                                           \
                                                                               \
  TMAP_COMPARE_FUNC_SIG(KeyT, KeyName);

#define TMAP_ASSERT_SHOULD_USE_INSERT(val)                                     \
  STATIC_ASSERT(sizeof(val) <= 8, "only small values should use insert, use "  \
                                  "insert slot for larger ones instead!")

#define TMAP_ASSERT_SHOULD_USE_INSERT_SLOT(val)                                \
  STATIC_ASSERT(sizeof(val) > 8, "only big values should use insert slot, "    \
                                 "use insert for smaller ones instead!")

#define TMAP_PUT(Name, Map, Key, Value) tmap_put_##Name(Map, Key, Value)
#define TMAP_PUT_SLOT(Name, Map, Key) tmap_put_slot_##Name(Map, Key)
#define TMAP_INSERT(Name, Map, Key, Value, AllowOverwrite)                     \
  tmap_insert_##Name(Map, Key, Value, AllowOverwrite)
#define TMAP_INSERT_SLOT(Name, Map, Key, AllowOverwrite)                       \
  tmap_insert_slot_##Name(Map, Key, AllowOverwrite)
#define TMAP_GET(Name, Map, Key) tmap_get_##Name(Map, Key)
#define TMAP_GET_ENTRY(Name, Map, Key) tmap_get_entry_##Name(Map, Key)
#define TMAP_GET_MUT(Name, Map, Key) tmap_get_mut_##Name(Map, Key)
#define TMAP_GET_ENTRY_AT(Name, Map, Index) tmap_get_entry_at_##Name(Map, Index)
#define TMAP_REM(Name, Map, Key) tmap_remove_##Name(Map, Key)
#define TMAP_FREE(Name, Map) tmap_free_##Name(Map)
#define TMAP_CLEAR(Name, Map) tmap_clear_##Name(Map)

#define TMAP_INIT(Name) tmap_init_##Name()

#define TMAP_EMPTY_MAP(TypeName)                                               \
  ((TMAP_TYPENAME_MAP(TypeName)){                                              \
      .buckets = NULL, .capacity = 0, .count = 0, .occupied = 0})

#define TMAP_SIZE(v) (v).count
#define TMAP_IS_EMPTY(v) ((v).count == 0)
#define TMAP_CAPACITY(v) (v).capacity
#define TMAP_OCCUPIED_COUNT(v) (v).occupied

#if defined(Z_HAS_CLEANUP) && Z_HAS_CLEANUP
#define tmap_autofree(Name) Z_CLEANUP(map_free_##Name) TMAP_TYPENAME_MAP(Name)
#endif

#define TMAP_IMPLEMENT_MAP_TYPE(KeyT, KeyName, ValT, Name)                     \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_free_##Name(TMAP_TYPENAME_MAP(Name) * m) {     \
    Z_MAP_FREE(m->buckets);                                                    \
    *m = (TMAP_TYPENAME_MAP(Name)){0};                                         \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES TMAP_TYPENAME_MAP(Name) tmap_init_##Name(void) {         \
    return (TMAP_TYPENAME_MAP(Name)){                                          \
        .buckets = NULL, .capacity = 0, .count = 0, .occupied = 0};            \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES TmapResult tmap_resize_##Name(                           \
      TMAP_TYPENAME_MAP(Name) * m, size_t new_cap) {                           \
    if (new_cap < m->occupied) {                                               \
      return TmapResultErr;                                                    \
    }                                                                          \
    TMAP_TYPENAME_BUCKET(Name) *new_buckets =                                  \
        Z_MAP_CALLOC(new_cap, sizeof(TMAP_TYPENAME_BUCKET(Name)));             \
    if (!new_buckets) {                                                        \
      return TmapResultErr;                                                    \
    };                                                                         \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      if (m->buckets[i].state == TMAP_OCCUPIED) {                              \
        TmapHashType hash =                                                    \
            TMAP_HASH_FUNC_NAME(KeyName)(m->buckets[i].entry.key);             \
        size_t idx = hash % new_cap;                                           \
        while (new_buckets[idx].state == TMAP_OCCUPIED) {                      \
          idx = (idx + 1) % new_cap;                                           \
        }                                                                      \
        new_buckets[idx] = m->buckets[i];                                      \
      }                                                                        \
    }                                                                          \
    Z_MAP_FREE(m->buckets);                                                    \
    m->buckets = new_buckets;                                                  \
    m->capacity = new_cap;                                                     \
    m->occupied = m->count;                                                    \
    return TmapResultOk;                                                       \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES ValT *tmap_insert_slot_##Name(                           \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, bool allow_overwrite) {           \
    if (m->occupied >= m->capacity * 0.75) {                                   \
      size_t new_cap = m->capacity == 0 ? 16 : m->capacity * 2;                \
      if (tmap_resize_##Name(m, new_cap) != TmapResultOk) {                    \
        return NULL;                                                           \
      }                                                                        \
    }                                                                          \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
    size_t deleted_idx = SIZE_MAX;                                             \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == TMAP_EMPTY) {                                                   \
        if (deleted_idx != SIZE_MAX)                                           \
          idx = deleted_idx;                                                   \
        else                                                                   \
          m->occupied++;                                                       \
        m->buckets[idx] = (TMAP_TYPENAME_BUCKET(Name)){                        \
            .entry =                                                           \
                (TMAP_TYPENAME_ENTRY(Name)){.key = key, .value = (ValT){}},    \
            .state = TMAP_OCCUPIED};                                           \
        m->count++;                                                            \
        return &(m->buckets[idx].entry.value);                                 \
      }                                                                        \
      if (s == TMAP_DELETED) {                                                 \
        if (deleted_idx == SIZE_MAX)                                           \
          deleted_idx = idx;                                                   \
      } else if (TMAP_COMPARE_FUNC_NAME(KeyName)(m->buckets[idx].entry.key,    \
                                                 key) == 0) {                  \
        if (!allow_overwrite) {                                                \
          return TMAP_WOULD_OVERWRITE;                                         \
        }                                                                      \
        return &(m->buckets[idx].entry.value);                                 \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES TmapInsertResult tmap_insert_##Name(                     \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, ValT const val,                   \
      bool allow_overwrite) {                                                  \
    ValT *slot = tmap_insert_slot_##Name(m, key, allow_overwrite);             \
    if (slot == NULL) {                                                        \
      return TmapInsertResultErr;                                              \
    }                                                                          \
    if (slot == TMAP_WOULD_OVERWRITE) {                                        \
      return TmapInsertResultWouldOverwrite;                                   \
    }                                                                          \
    *slot = val;                                                               \
    return TmapInsertResultOk;                                                 \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES ValT *tmap_get_mut_##Name(TMAP_TYPENAME_MAP(Name) * m,   \
                                                const KeyT key) {              \
    if (m->count == 0)                                                         \
      return NULL;                                                             \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == TMAP_EMPTY)                                                     \
        return NULL;                                                           \
      if (s == TMAP_OCCUPIED && TMAP_COMPARE_FUNC_NAME(KeyName)(               \
                                    m->buckets[idx].entry.key, key) == 0) {    \
        return &(m->buckets[idx].entry.value);                                 \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES ValT const *tmap_get_##Name(                             \
      TMAP_TYPENAME_MAP(Name) const *const m, const KeyT key) {                \
    if (m->count == 0)                                                         \
      return NULL;                                                             \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == TMAP_EMPTY)                                                     \
        return NULL;                                                           \
      if (s == TMAP_OCCUPIED && TMAP_COMPARE_FUNC_NAME(KeyName)(               \
                                    m->buckets[idx].entry.key, key) == 0) {    \
        return &(m->buckets[idx].entry.value);                                 \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES const TMAP_TYPENAME_ENTRY(Name) *                        \
      tmap_get_entry_##Name(const TMAP_TYPENAME_MAP(Name) *const m,            \
                            const KeyT key) {                                  \
    if (m->count == 0)                                                         \
      return NULL;                                                             \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == TMAP_EMPTY)                                                     \
        return NULL;                                                           \
      if (s == TMAP_OCCUPIED && TMAP_COMPARE_FUNC_NAME(KeyName)(               \
                                    m->buckets[idx].entry.key, key) == 0) {    \
        return &(m->buckets[idx].entry);                                       \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] const TMAP_TYPENAME_ENTRY(Name) *          \
      tmap_get_entry_at_##Name(const TMAP_TYPENAME_MAP(Name) *const m,         \
                               size_t index) {                                 \
    if (index >= m->capacity) {                                                \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    TMAP_TYPENAME_BUCKET(Name) *hm_bucket = &(m->buckets[index]);              \
                                                                               \
    if (hm_bucket->state == TMAP_OCCUPIED) {                                   \
      return &(hm_bucket->entry);                                              \
    }                                                                          \
                                                                               \
    return TMAP_NO_ELEMENT_HERE;                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_remove_##Name(TMAP_TYPENAME_MAP(Name) * m,     \
                                              const KeyT key) {                \
    if (m->count == 0)                                                         \
      return;                                                                  \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == TMAP_EMPTY)                                                     \
        return;                                                                \
      if (s == TMAP_OCCUPIED && TMAP_COMPARE_FUNC_NAME(KeyName)(               \
                                    m->buckets[idx].entry.key, key) == 0) {    \
        m->buckets[idx].state = TMAP_DELETED;                                  \
        m->count--;                                                            \
        return;                                                                \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_clear_##Name(TMAP_TYPENAME_MAP(Name) * m) {    \
    if (m->capacity > 0) {                                                     \
      memset(m->buckets, 0, m->capacity * sizeof(TMAP_TYPENAME_BUCKET(Name))); \
    }                                                                          \
    m->count = 0;                                                              \
    m->occupied = 0;                                                           \
  }

#define TMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(KeyT, KeyName, ValT, Name)          \
  TMAP_DEFINE_MAP_TYPE(KeyT, KeyName, ValT, Name)                              \
  TMAP_IMPLEMENT_MAP_TYPE(KeyT, KeyName, ValT, Name)

#ifdef __cplusplus
// extern "C" {
}
#endif
