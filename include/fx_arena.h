#ifndef FX_ARENA_H
#define FX_ARENA_H

#include <stdint.h>

typedef struct fx_slice_t{
    uint64_t size;
    uint8_t* ptr;
} fx_slice_t ;

typedef struct fx_arena_t {
    uint64_t capacity;
    fx_slice_t mem;
} fx_arena_t ;

#define DECLARE_SLICE(name, type) \
    const uint8_t FX_SLICE_##name##_STRIDE = sizeof type \
    fx_slice_t name

#define SLICE_LEN(name) \
    name.size / FX_SLICE_##name##_STRIDE

#define SLICE_GET(name, index) \
    name.ptr[ index * FX_SLICE_##name##_STRIDE ]


fx_arena_t fx_arena_create();

void fx_arena_destroy(fx_arena_t* arena);

fx_slice_t fx_arena_alloc(fx_arena_t* arena, uint64_t amount);

void fx_arena_free(fx_arena_t* arena);

#endif
