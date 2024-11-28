#pragma once

#include "Allocator.hh"

#include <cassert>
#include <cstdlib>

namespace adt
{

struct OsAllocator
{
    Allocator super {};

    constexpr OsAllocator([[maybe_unused]] u32 _ingnored = 0);

    [[nodiscard]] void* alloc(u64 mCount, u64 mSize);
    [[nodiscard]] void* zalloc(u64 mCount, u64 mSize);
    [[nodiscard]] void* realloc(void* ptr, u64 mCount, u64 mSize);
    void free(void* ptr);
    void freeAll();
};

inline void*
OsAllocator::alloc(u64 mCount, u64 mSize)
{
    auto* r = ::malloc(mCount * mSize);
    assert(r != nullptr && "[OsAllocator]: calloc failed");
    return r;
}

inline void*
OsAllocator::zalloc(u64 mCount, u64 mSize)
{
    auto* r = ::calloc(mCount, mSize);
    assert(r != nullptr && "[OsAllocator]: calloc failed");
    return r;
}

inline void*
OsAllocator::realloc(void* p, u64 mCount, u64 mSize)
{
    auto* r = ::realloc(p, mCount * mSize);
    assert(r != nullptr && "[OsAllocator]: realloc failed");
    return r;
}

inline void
OsAllocator::free(void* p)
{
    ::free(p);
}

inline void
OsAllocator::freeAll()
{
    assert(false && "[OsAllocator]: can't 'freeAll()'");
}

inline const AllocatorVTable inl_OsAllocatorVTable {
    .alloc = decltype(AllocatorVTable::alloc)(+[](OsAllocator* s, u64 mCount, u64 mSize) { return s->alloc(mCount, mSize); }),
    .zalloc = decltype(AllocatorVTable::zalloc)(+[](OsAllocator* s, u64 mCount, u64 mSize) { return s->zalloc(mCount, mSize); }),
    .realloc = decltype(AllocatorVTable::realloc)(+[](OsAllocator* s, void* ptr, u64 mCount, u64 mSize) { return s->realloc(ptr, mCount, mSize); }),
    .free = decltype(AllocatorVTable::free)(+[](OsAllocator* s, void* ptr) { s->free(ptr); }),
    .freeAll = decltype(AllocatorVTable::freeAll)(+[](OsAllocator* s) { s->freeAll(); } ),
};

constexpr
OsAllocator::OsAllocator([[maybe_unused]] u32 _ingnored) : super(&inl_OsAllocatorVTable) {}

inline OsAllocator inl_OsAllocator {};
inline Allocator* inl_pOsAlloc = &inl_OsAllocator.super;

} /* namespace adt */
