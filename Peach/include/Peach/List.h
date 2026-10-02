//
// Created by Štěpán Toman on 02.10.2026.
//

#ifndef PEACH_LIST_H
#define PEACH_LIST_H
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    size_t capacity;
    size_t size;
} vec_header;

#define vec__header(v) ((vec_header *)(v) - 1)

#define mListSize(v)     ((v) ? vec__header(v)->size : 0)
#define mListCapacity(v) ((v) ? vec__header(v)->capacity : 0)

#define mListFree(v)     do { if (v) { free(vec__header(v)); (v) = NULL; } } while (0)

#define mListPush(v, val) \
    do { \
        if (!(v) || vec__header(v)->size >= vec__header(v)->capacity) { \
            vec__grow((void **)&(v), sizeof(*(v))); \
        } \
        (v)[vec__header(v)->size++] = (val); \
    } while (0)

#define mListPop(v)      ((v)[--vec__header(v)->size])

static inline void vec__grow(void **v, size_t item_size) {
    size_t new_cap = *v ? vec__header(*v)->capacity * 2 : 8;
    vec_header *p = (vec_header *)realloc(*v ? vec__header(*v) : NULL,
                                          sizeof(vec_header) + new_cap * item_size);
    p->capacity = new_cap;
    if (!*v) p->size = 0;
    *v = p + 1; // Return the pointer to the payload, hiding the header
}

#endif //PEACH_LIST_H
