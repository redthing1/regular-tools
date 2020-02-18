#include <stdint.h>
#include <stdlib.h>

#define DECLARE_BUFFIE_STRUCT(type)                                                                                    \
    typedef struct Buffie_##type {                                                                                     \
        ##type *buf;                                                                                                   \
        size_t ct;                                                                                                     \
        size_t sz;                                                                                                     \
    } Buffie_##type;

#define DECLARE_BUFFIE_FUNCS(type)                                                                                     \
    void buf_alloc_##type(Buffie_##type *b, size_t count) {                                                            \
        b->sz = count;                                                                                                 \
        b->buf = malloc(count * sizeof(##type));                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    void buf_set(Buffie_##type *b, size_t i, ##type val) { b->buf[i] = val; }                                          \
    ##type buf_get(Buffie_##type *b, size_t i) { return b->buf[i]; }                                                   \
                                                                                                                       \
    void buf_push_back(Buffie_##type *b, ##type val) {                                                                 \
        b->ct++;                                                                                                       \
        if (b->sz <= b->ct) {                                                                                          \
            b->sz = b->sz * 2;                                                                                         \
            b->buf = realloc(b->buf, b->sz * sizeof(##type));                                                          \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void buf_pop_back(Buffie_##type *b) { b->ct--; }                                                                   \
                                                                                                                       \
    void buf_free(Buffie_##type *b) {                                                                                  \
        free(b->buf);                                                                                                  \
        b->ct = 0;                                                                                                     \
        b->sz = 0;                                                                                                     \
        b->buf = NULL;                                                                                                 \
    }

#define BUFFIE_OF(type)                                                                                                \
    DECLARE_BUFFIE_STRUCT(##type)                                                                                      \
    DECLARE_BUFFIE_FUNCS(##type)
