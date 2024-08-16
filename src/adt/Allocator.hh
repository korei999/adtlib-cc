#pragma once

#include "types.hh"

namespace adt
{

constexpr u64 align8(u64 x) { return ((x) + 8 - 1) & (~(8 - 1)); }
constexpr u64 align(u64 x, u64 to) { return ((x) + to - 1) & (~(to - 1)); }

constexpr u64 SIZE_MIN = 2UL;
constexpr u64 SIZE_1K = 1024UL;
constexpr u64 SIZE_8K = 8UL * SIZE_1K;
constexpr u64 SIZE_1M = SIZE_1K * SIZE_1K; 
constexpr u64 SIZE_8M = 8UL * SIZE_1M; 
constexpr u64 SIZE_1G = SIZE_1M * SIZE_1K; 

struct Allocator
{
    struct Interface
    {
        void* (*alloc)(Allocator* s, u64 mCount, u64 mSize);
        void* (*realloc)(Allocator* s, void* p, u64 mCount, u64 mSize);
        void (*free)(Allocator* s, void* p);
    };

    /* allow non const? */
    const Interface* pVTable;
};

constexpr void* alloc(Allocator* s, u64 mCount, u64 mSize) { return s->pVTable->alloc(s, mCount, mSize); }
constexpr void* realloc(Allocator* s, void* p, u64 mCount, u64 mSize) { return s->pVTable->realloc(s, p, mCount, mSize); }
constexpr void free(Allocator* s, void* p) { s->pVTable->free(s, p); }

} /* namespace adt */
