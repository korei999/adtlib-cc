#pragma once

#include "IAllocator.hh"

#include <cassert>
#include <cstdlib>

namespace adt
{

/* default os allocator (aka malloc() / calloc() / realloc() / free()).
 * freeAll() method is not supported. */
struct OsAllocator;

inline void* OsAlloc(OsAllocator* s, u64 mCount, u64 mSize);
inline void* OsZalloc(OsAllocator* s, u64 mCount, u64 mSize);
inline void* OsRealloc(OsAllocator* s, void* p, u64 mCount, u64 mSize);
inline void OsFree(OsAllocator* s, void* p);
inline void _OsFreeAll(OsAllocator* s);

inline void* alloc(OsAllocator* s, u64 mCount, u64 mSize) { return OsAlloc(s, mCount, mSize); }
inline void* zalloc(OsAllocator* s, u64 mCount, u64 mSize) { return OsZalloc(s, mCount, mSize); }
inline void* realloc(OsAllocator* s, void* p, u64 mCount, u64 mSize) { return OsRealloc(s, p, mCount, mSize); }
inline void free(OsAllocator* s, void* p) { OsFree(s, p); }

inline const AllocatorVTable inl_OsAllocatorVTable {
    .alloc = decltype(AllocatorVTable::alloc)(OsAlloc),
    .zalloc = decltype(AllocatorVTable::zalloc)(OsZalloc),
    .realloc = decltype(AllocatorVTable::realloc)(OsRealloc),
    .free = decltype(AllocatorVTable::free)(OsFree),
    .freeAll = decltype(AllocatorVTable::freeAll)(_OsFreeAll),
};

struct OsAllocator
{
    IAllocator super {};

    constexpr OsAllocator([[maybe_unused]] u64 _ingnored = 0) : super(&inl_OsAllocatorVTable) {}
};

inline IAllocator*
OsAllocatorGet()
{
    static OsAllocator alloc {};
    return &alloc.super;
}

inline void*
OsAlloc(OsAllocator*, u64 mCount, u64 mSize)
{
    auto* r = ::malloc(mCount * mSize);
    assert(r != nullptr && "[OsAllocator]: calloc failed");
    return r;
}

inline void*
OsZalloc(OsAllocator*, u64 mCount, u64 mSize)
{
    auto* r = ::calloc(mCount, mSize);
    assert(r != nullptr && "[OsAllocator]: calloc failed");
    return r;
}

inline void*
OsRealloc(OsAllocator*, void* p, u64 mCount, u64 mSize)
{
    auto* r = ::realloc(p, mCount * mSize);
    assert(r != nullptr && "[OsAllocator]: realloc failed");
    return r;
}

inline void
OsFree(OsAllocator*, void* p)
{
    ::free(p);
}

inline void
_OsFreeAll(OsAllocator*)
{
    assert(false && "[OsAllocator]: no 'freeAll()' method");
}

} /* namespace adt */
