#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_INITIAL_CAPACITY 8

#define VECTOR_DECLARE(T, NAME, FN)                                            \
  typedef struct {                                                             \
    T *data;                                                                   \
    size_t size;                                                               \
    size_t capacity;                                                           \
  } Vec##NAME;                                                                 \
                                                                               \
  Vec##NAME *vec_init_##FN##_with_capacity(size_t capacity);                   \
  Vec##NAME *vec_init_##FN(void);                                              \
  void vec_free_##FN(Vec##NAME *v);                                            \
  int vec_reserve_##FN(Vec##NAME *v, size_t capacity);                         \
  int vec_push_##FN(Vec##NAME *v, T value);                                    \
  T *vec_pop_##FN(Vec##NAME *v, size_t id);                                    \
  T *vec_at_##FN(Vec##NAME *v, size_t id);

#define VECTOR_DEFINE(T, NAME, FN)                                             \
  Vec##NAME *vec_init_##FN##_with_capacity(size_t capacity) {                  \
    Vec##NAME *v = malloc(sizeof(*v));                                         \
    if (!v)                                                                    \
      return NULL;                                                             \
    v->size = 0;                                                               \
    if (capacity > 0) {                                                        \
      v->data = malloc(capacity * sizeof(T));                                  \
      if (!v->data) {                                                          \
        free(v);                                                               \
        return NULL;                                                           \
      }                                                                        \
      v->capacity = capacity;                                                  \
    } else {                                                                   \
      v->data = NULL;                                                          \
      v->capacity = 0;                                                         \
    }                                                                          \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  Vec##NAME *vec_init_##FN(void) { return vec_init_##FN##_with_capacity(0); }  \
                                                                               \
  void vec_free_##FN(Vec##NAME *v) {                                           \
    if (!v)                                                                    \
      return;                                                                  \
    free(v->data);                                                             \
    free(v);                                                                   \
  }                                                                            \
                                                                               \
  int vec_reserve_##FN(Vec##NAME *v, size_t capacity) {                        \
    if (!v)                                                                    \
      return -1;                                                               \
    if (capacity <= v->capacity)                                               \
      return 0;                                                                \
    T *p = realloc(v->data, capacity * sizeof(T));                             \
    if (!p)                                                                    \
      return -1;                                                               \
    v->data = p;                                                               \
    v->capacity = capacity;                                                    \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  int vec_push_##FN(Vec##NAME *v, T value) {                                   \
    if (!v)                                                                    \
      return -1;                                                               \
    if (v->size == v->capacity) {                                              \
      size_t new_capacity =                                                    \
          v->capacity ? v->capacity * 2 : VECTOR_INITIAL_CAPACITY;             \
      if (vec_reserve_##FN(v, new_capacity) != 0)                              \
        return -1;                                                             \
    }                                                                          \
    v->data[v->size++] = value;                                                \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  T *vec_pop_##FN(Vec##NAME *v, size_t id) {                                   \
    if (!v)                                                                    \
      return NULL;                                                             \
    if (id >= v->size)                                                         \
      return NULL;                                                             \
    T *value = malloc(sizeof(T));                                              \
    if (!value)                                                                \
      return NULL;                                                             \
    *value = v->data[id];                                                      \
    memmove(&v->data[id], &v->data[id + 1], (v->size - id - 1) * sizeof(T));   \
    v->size--;                                                                 \
    return value;                                                              \
  }                                                                            \
                                                                               \
  T *vec_at_##FN(Vec##NAME *v, size_t id) {                                    \
    if (!v || id >= v->size)                                                   \
      return NULL;                                                             \
    return &v->data[id];                                                       \
  }

#endif /* VEC_H */
