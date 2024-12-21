#pragma once

#include "IAllocator.hh"

#include <cassert>
#include <cstdlib>

namespace adt
{

/* default os allocator (aka malloc() / calloc() / realloc() / free()).
 * freeAll() method is not supported. */
struct OsAllocator : IAllocator
{
    [[nodiscard]] virtual void* alloc(u64 mCount, u64 mSize) override final;
    [[nodiscard]] virtual void* zalloc(u64 mCount, u64 mSize) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, u64 mCount, u64 mSize) override final;
    void virtual free(void* ptr) override final;
    void virtual freeAll() override final; /* assert(false) */
};

inline IAllocator*
OsAllocatorGet()
{
    static OsAllocator alloc {};
    return &alloc;
}

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
    assert(false && "[OsAllocator]: no 'freeAll()' method");
}

} /* namespace adt */
