#include <stdint.h>
#include <stdlib.h>

#define DECLARE_BUFFIE(type) BUFFIE_##type

struct ElemBuffie {
    Elem *buf;
    size_t ct;
    size_t sz;
};

void buf_allocate(ElemBuffie *b, size_t count) {
    b->sz = count;
    b->buf = malloc(count * sizeof(Elem));
}

void buf_set(ElemBuffie *b, size_t i, Elem val) { b->buf[i] = val; }

Elem buf_get(ElemBuffie *b, size_t i) { return b->buf[i]; }

void buf_push_back(ElemBuffie *b, Elem val) {
    b->ct++;
    if (b->sz <= b->ct) {
        // reallocate the buffer
        b->sz = b->sz * 2;
        b->buf = realloc(b->buf, b->sz * sizeof(Elem));
    }
}

void buf_pop_back(ElemBuffie *b) {
    b->ct--;
    // we don't really clear the memory or anything
}

void buf_free(ElemBuffie *b) {
    free(b->buf);
    b->ct = 0;
    b->sz = 0;
    b->buf = NULL;
}
