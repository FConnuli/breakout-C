#include "raylib.h"
#include <stdint.h>
#include "fx_arena.h"

#define STARTING_CAPACITY 4096

fx_arena_t fx_arena_create() {
    fx_arena_t arena = {
        .mem.size = 0,
        .capacity = STARTING_CAPACITY,
        .mem.ptr = MemAlloc(STARTING_CAPACITY),
    };
    return arena;
}

void fx_arena_destroy(fx_arena_t* arena) {
    arena->mem.size = 0;
    arena->capacity = 0;
    MemFree(arena->mem.ptr);
}

void fx_arena_free(fx_arena_t* arena){
    arena->mem.size = 0;
}

fx_slice_t fx_arena_alloc(fx_arena_t* arena, uint64_t amount) {
    bool cap_raised = false;
    uint64_t old_size = arena->mem.size;
    arena->mem.size += amount;
    while (arena->mem.size > arena->capacity) {
        cap_raised = true;
        arena->capacity *= 2;
    }
    if (cap_raised) {
        arena->mem.ptr = MemRealloc(arena->mem.ptr, arena->capacity);
    }
    fx_slice_t allocation = {
        .ptr = arena->mem.ptr + old_size,
        .size = amount,
    };
    return allocation;
    
}
