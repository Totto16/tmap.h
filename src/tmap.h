/*
 * tmap.h
 *
 * This file is part of the z-libs collection: https://github.com/z-libs
 * Licensed under the MIT License.
 *
 * based on
 * https://github.com/z-libs/zmap.h/blob/d153fb0ffbcb2992e8ff8382e4f9462501c63bd6/zmap.h
 *
 * modified to suit my needs
 *
 * Note: newest version at the moment is
 * https://github.com/z-libs/zmap.h/blob/46844cae134af17202d9a97818f3a1c78893f41e/zmap.h
 * (1.1.0) but it uses another internal representation of the map
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zmap.h
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

typedef enum : uint8_t {
  TmapInsertResultOk = 0,
  TmapInsertResultErr = 1,
  TmapInsertResultWouldOverwrite = 2
} TmapInsertResult;

// Memory Macros.
// If the user hasn't defined their own allocator, use the standard one.
#ifndef T_MALLOC
#define T_MALLOC(sz) malloc(sz)
#define T_CALLOC(n, sz) calloc(n, sz)
#define T_REALLOC(p, sz) realloc(p, sz)
#define T_FREE(p) free(p)
#endif

#ifndef T_MAP_MALLOC
#define T_MAP_MALLOC(sz) T_MALLOC(sz)
#endif
#ifndef T_MAP_CALLOC
#define T_MAP_CALLOC(n, sz) T_CALLOC(n, sz)
#endif
#ifndef T_MAP_FREE
#define T_MAP_FREE(p) T_FREE(p)
#endif

// Growth strategy.

/* * Determines how containers expand when full.
 * Default is 2.0x (Geometric Growth).
 *
 * Optimization note:
 * 2.0x minimizes realloc calls but can waste memory.
 * 1.5x is often better for memory fragmentation and reuse.
 */
#ifndef T_GROWTH_FACTOR
// Default: Double capacity (2.0x).
#define T_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) * 2)

// Alternative: 1.5x Growth (Uncomment to use in your project).
// #define T_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) + (cap) / 2)
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) ||              \
    defined(__cplusplus)
#define STATIC_ASSERT(check, message) static_assert(check, message)
#define MAYBE_UNUSED [[maybe_unused]]
#elif __STDC_VERSION__ < 201112L
#define MAYBE_UNUSED __attribute__((unused))
// empty, as not supported
#define STATIC_ASSERT(check, message)
#else
#define MAYBE_UNUSED [[maybe_unused]]
#define STATIC_ASSERT(check, message) _Static_assert(check, message)
#endif

// maybe some visibility things later, but I just removed the
// TMAP_FUN_ATTRIBUTES
#define TMAP_FUN_ATTRIBUTES

#define TMAP_STATIC_INLINE MAYBE_UNUSED static inline

typedef size_t TmapHashType;

TMAP_FUN_ATTRIBUTES TmapHashType tmap_default_hash(const void *key, size_t len);

TMAP_FUN_ATTRIBUTES TmapHashType tmap_stbds_hash_string(const char *str);

TMAP_FUN_ATTRIBUTES TmapHashType tmap_stbds_hash_bytes(const void *key,
                                                       size_t len);

typedef enum {
  tmap_state_empty = 0,
  tmap_state_occupied,
  tmap_state_deleted,
} tmap_state;

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

// Default load factor (0.75 is standard for open addressing).
#define TMAP_LOAD_FACTOR 0.75f

#define TMAP_TYPENAME_ENTRY(TypeName) tmap_entry_##TypeName

#define TMAP_TYPENAME_BUCKET(TypeName) tmap_bucket_##TypeName

#define TMAP_TYPENAME_MAP(TypeName) tmap_##TypeName

#define TMAP_TYPENAME_ITER(TypeName) tmap_iter_##TypeName

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
    size_t count;     /* Number of active items. */                            \
    size_t occupied;  /* Active + Deleted items (for load factor). */          \
    size_t threshold; /* Grow when occupied >= threshold. */                   \
  } TMAP_TYPENAME_MAP(Name);                                                   \
  /* Iterator state struct. */                                                 \
  typedef struct {                                                             \
    const TMAP_TYPENAME_MAP(Name) * map;                                       \
    size_t index;                                                              \
  } TMAP_TYPENAME_ITER(Name);                                                  \
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
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_STATIC_INLINE ValT *                  \
  tmap_put_slot_##Name(TMAP_TYPENAME_MAP(Name) * m, KeyT key) {                \
    return tmap_insert_slot_##Name(m, key, true);                              \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_STATIC_INLINE TmapResult              \
  tmap_put_##Name(TMAP_TYPENAME_MAP(Name) * m, KeyT key, ValT const val) {     \
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
  TMAP_FUN_ATTRIBUTES void tmap_remove_##Name(TMAP_TYPENAME_MAP(Name) * m,     \
                                              const KeyT key);                 \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_TYPENAME_ITER(Name)                   \
      tmap_iter_init_##Name(TMAP_TYPENAME_MAP(Name) const *const m);           \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] bool tmap_iter_next_##Name(                \
      TMAP_TYPENAME_ITER(Name) * it, TMAP_TYPENAME_ENTRY(Name) * out_entry);   \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_STATIC_INLINE bool                    \
  tmap_contains_##Name(TMAP_TYPENAME_MAP(Name) const *const m,                 \
                       const KeyT key) {                                       \
    return tmap_get_##Name(m, key) != NULL;                                    \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_STATIC_INLINE size_t                  \
  tmap_size_##Name(TMAP_TYPENAME_MAP(Name) const *const m) {                   \
    return m->count;                                                           \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES [[nodiscard]] TMAP_STATIC_INLINE bool                    \
  tmap_is_empty_##Name(TMAP_TYPENAME_MAP(Name) const *const m) {               \
    return m->count == 0;                                                      \
  }                                                                            \
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
#define TMAP_REM(Name, Map, Key) tmap_remove_##Name(Map, Key)
#define TMAP_FREE(Name, Map) tmap_free_##Name(Map)
#define TMAP_CLEAR(Name, Map) tmap_clear_##Name(Map)
#define TMAP_SIZE(Name, Map) tmap_size_##Name(Map)
#define TMAP_IS_EMPTY(Name, Map) tmap_is_empty_##Name(Map)
#define TMAP_ITER_INIT(Name, Map) tmap_iter_init_##Name(Map)
#define TMAP_ITER_NEXT(Name, Map, OutPtr) tmap_iter_next_##Name(Map, OutPtr)

#define TMAP_INIT(Name) tmap_init_##Name()

#define TMAP_EMPTY(TypeName)                                                   \
  ((TMAP_TYPENAME_MAP(TypeName)){.buckets = NULL,                              \
                                 .capacity = 0,                                \
                                 .count = 0,                                   \
                                 .occupied = 0,                                \
                                 .threshold = 0})

#define TMAP_IMPLEMENT_MAP_TYPE(KeyT, KeyName, ValT, Name)                     \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_free_##Name(TMAP_TYPENAME_MAP(Name) * m) {     \
    T_MAP_FREE(m->buckets);                                                    \
    *m = (TMAP_TYPENAME_MAP(Name)){0};                                         \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES TMAP_TYPENAME_MAP(Name) tmap_init_##Name(void) {         \
    return (TMAP_TYPENAME_MAP(Name)){.buckets = NULL,                          \
                                     .capacity = 0,                            \
                                     .count = 0,                               \
                                     .occupied = 0,                            \
                                     .threshold = 0};                          \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES TmapResult tmap_resize_##Name(                           \
      TMAP_TYPENAME_MAP(Name) * m, size_t new_cap) {                           \
    if (new_cap < m->occupied) {                                               \
      return TmapResultErr;                                                    \
    }                                                                          \
    TMAP_TYPENAME_BUCKET(Name) *new_buckets = (TMAP_TYPENAME_BUCKET(           \
        Name) *)T_MAP_CALLOC(new_cap, sizeof(TMAP_TYPENAME_BUCKET(Name)));     \
    if (!new_buckets) {                                                        \
      return TmapResultErr;                                                    \
    };                                                                         \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      if (m->buckets[i].state == tmap_state_occupied) {                        \
        TmapHashType hash =                                                    \
            TMAP_HASH_FUNC_NAME(KeyName)(m->buckets[i].entry.key);             \
        size_t idx = hash % new_cap;                                           \
        while (new_buckets[idx].state == tmap_state_occupied) {                \
          idx = (idx + 1) % new_cap;                                           \
        }                                                                      \
        new_buckets[idx] = m->buckets[i];                                      \
      }                                                                        \
    }                                                                          \
    T_MAP_FREE(m->buckets);                                                    \
    m->buckets = new_buckets;                                                  \
    m->capacity = new_cap;                                                     \
    m->occupied = m->count; /* Deleted items are purged during resize. */      \
    m->threshold = (size_t)(new_cap * TMAP_LOAD_FACTOR);                       \
    return TmapResultOk;                                                       \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES ValT *tmap_insert_slot_##Name(                           \
      TMAP_TYPENAME_MAP(Name) * m, KeyT key, bool allow_overwrite) {           \
    if (m->occupied >= m->threshold) {                                         \
      size_t new_cap = T_GROWTH_FACTOR(m->capacity);                           \
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
      if (s == tmap_state_empty) {                                             \
        if (deleted_idx != SIZE_MAX) {                                         \
          idx = deleted_idx;                                                   \
        } else {                                                               \
          m->occupied++;                                                       \
        }                                                                      \
        m->buckets[idx] = (TMAP_TYPENAME_BUCKET(Name)){                        \
            .entry =                                                           \
                (TMAP_TYPENAME_ENTRY(Name)){.key = key, .value = (ValT){}},    \
            .state = tmap_state_occupied};                                     \
        m->count++;                                                            \
        return &(m->buckets[idx].entry.value);                                 \
      } else if (s == tmap_state_deleted) {                                    \
        if (deleted_idx == SIZE_MAX) {                                         \
          deleted_idx = idx;                                                   \
        }                                                                      \
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
    } else if (slot == TMAP_WOULD_OVERWRITE) {                                 \
      return TmapInsertResultWouldOverwrite;                                   \
    }                                                                          \
    *slot = val;                                                               \
    return TmapInsertResultOk;                                                 \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES ValT *tmap_get_mut_##Name(TMAP_TYPENAME_MAP(Name) * m,   \
                                                const KeyT key) {              \
    if (m->count == 0) {                                                       \
      return NULL;                                                             \
    }                                                                          \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == tmap_state_empty) {                                             \
        return NULL;                                                           \
      } else if (s == tmap_state_occupied &&                                   \
                 TMAP_COMPARE_FUNC_NAME(KeyName)(m->buckets[idx].entry.key,    \
                                                 key) == 0) {                  \
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
    if (m->count == 0) {                                                       \
      return NULL;                                                             \
    }                                                                          \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == tmap_state_empty) {                                             \
        return NULL;                                                           \
      } else if (s == tmap_state_occupied &&                                   \
                 TMAP_COMPARE_FUNC_NAME(KeyName)(m->buckets[idx].entry.key,    \
                                                 key) == 0) {                  \
        return &(m->buckets[idx].entry);                                       \
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
      if (s == tmap_state_empty) {                                             \
        return NULL;                                                           \
      } else if (s == tmap_state_occupied &&                                   \
                 TMAP_COMPARE_FUNC_NAME(KeyName)(m->buckets[idx].entry.key,    \
                                                 key) == 0) {                  \
        return &(m->buckets[idx].entry.value);                                 \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  TMAP_FUN_ATTRIBUTES void tmap_remove_##Name(TMAP_TYPENAME_MAP(Name) * m,     \
                                              const KeyT key) {                \
    if (m->count == 0) {                                                       \
      return;                                                                  \
    }                                                                          \
    TmapHashType hash = TMAP_HASH_FUNC_NAME(KeyName)(key);                     \
    size_t idx = hash % m->capacity;                                           \
                                                                               \
    for (size_t i = 0; i < m->capacity; i++) {                                 \
      tmap_state s = m->buckets[idx].state;                                    \
      if (s == tmap_state_empty)                                               \
        return;                                                                \
      if (s == tmap_state_occupied &&                                          \
          TMAP_COMPARE_FUNC_NAME(KeyName)(m->buckets[idx].entry.key, key) ==   \
              0) {                                                             \
        m->buckets[idx].state = tmap_state_deleted;                            \
        m->count--;                                                            \
        return;                                                                \
      }                                                                        \
      idx = (idx + 1) % m->capacity;                                           \
    }                                                                          \
  }                                                                            \
                                                                               \
  /* Creates a new iterator for the map. */                                    \
  TMAP_FUN_ATTRIBUTES TMAP_TYPENAME_ITER(Name)                                 \
      tmap_iter_init_##Name(TMAP_TYPENAME_MAP(Name) const *const m) {          \
    return (TMAP_TYPENAME_ITER(Name)){.map = m, .index = 0};                   \
  }                                                                            \
                                                                               \
  /* Advances the iterator. Returns true and writes key/val if a next item     \
   * exists. */                                                                \
  TMAP_FUN_ATTRIBUTES bool tmap_iter_next_##Name(                              \
      TMAP_TYPENAME_ITER(Name) *const it,                                      \
      TMAP_TYPENAME_ENTRY(Name) * out_entry) {                                 \
    if (!it->map || !it->map->buckets) {                                       \
      return false;                                                            \
    }                                                                          \
                                                                               \
    while (it->index < it->map->capacity) {                                    \
      size_t i = it->index++;                                                  \
      if (it->map->buckets[i].state == tmap_state_occupied) {                  \
        if (out_entry) {                                                       \
          *out_entry = it->map->buckets[i].entry;                              \
        }                                                                      \
        return true;                                                           \
      }                                                                        \
    }                                                                          \
    return false;                                                              \
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
