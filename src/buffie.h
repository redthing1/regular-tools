#include <stdint.h>
#include <stdlib.h>

#define DECLARE_BUFFIE_STRUCT(TYPE)                                                                                    \
    typedef struct Buffie_##TYPE {                                                                                     \
        TYPE *buf;                                                                                                     \
        size_t ct;                                                                                                     \
        size_t buf_sz;                                                                                                 \
    } Buffie_##TYPE;

#define DECLARE_BUFFIE_FUNCS(TYPE)                                                                                     \
    void buf_alloc_##TYPE(Buffie_##TYPE *b, size_t count) {                                                            \
        b->buf_sz = count;                                                                                             \
        b->ct = 0;                                                                                                     \
        b->buf = malloc(count * sizeof(TYPE));                                                                         \
    }                                                                                                                  \
                                                                                                                       \
    void buf_set_##TYPE(Buffie_##TYPE *b, size_t i, TYPE val) { b->buf[i] = val; }                                     \
    TYPE buf_get_##TYPE(Buffie_##TYPE *b, size_t i) { return b->buf[i]; }                                              \
                                                                                                                       \
    void buf_push_##TYPE(Buffie_##TYPE *b, TYPE val) {                                                                 \
        b->ct++;                                                                                                       \
        if (b->buf_sz <= b->ct) {                                                                                      \
            b->buf_sz = b->buf_sz * 2;                                                                                 \
            b->buf = realloc(b->buf, b->buf_sz * sizeof(TYPE));                                                        \
        }                                                                                                              \
        buf_set_##TYPE(b, b->ct - 1, val);                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    void buf_pop_##TYPE(Buffie_##TYPE *b) { b->ct--; }                                                                 \
                                                                                                                       \
    void buf_free_##TYPE(Buffie_##TYPE *b) {                                                                           \
        free(b->buf);                                                                                                  \
        b->ct = 0;                                                                                                     \
        b->buf_sz = 0;                                                                                                 \
        b->buf = NULL;                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    void buf_clear_##TYPE(Buffie_##TYPE *b) { b->ct = 0; }

#define BUFFIE_OF(TYPE)                                                                                                \
    DECLARE_BUFFIE_STRUCT(TYPE)                                                                                        \
    DECLARE_BUFFIE_FUNCS(TYPE)
