#pragma once

#include "Allocator.hh"
#include "utils.hh"

#include <cassert>
#include <cstring>

namespace adt
{

struct FixedAllocator
{
    Allocator base {};
    u8* pMemBuffer = nullptr;
    u64 size = 0;
    u64 cap = 0;
    void* pLastAlloc = nullptr;

    constexpr FixedAllocator() = default;
    constexpr FixedAllocator(void* pMemory, u64 capacity);
};

constexpr void* FixedAlloc(FixedAllocator* s, u64 mCount, u64 mSize);
constexpr void* FixedRealloc(FixedAllocator* s, void* p, u64 mCount, u64 mSize);
constexpr void FixedFree(FixedAllocator* s, void* p);
constexpr void FixedFreeAll(FixedAllocator* s);
constexpr void FixedReset(FixedAllocator* s);

inline void* alloc(FixedAllocator* s, u64 mCount, u64 mSize) { return FixedAlloc(s, mCount, mSize); }
inline void* realloc(FixedAllocator* s, void* p, u64 mCount, u64 mSize) { return FixedRealloc(s, p, mCount, mSize); }
inline void free(FixedAllocator* s, void* p) { return FixedFree(s, p); }
inline void freeAll(FixedAllocator* s) { return FixedFreeAll(s); }

constexpr void*
FixedAlloc(FixedAllocator* s, u64 mCount, u64 mSize)
{
    u64 aligned = align8(mCount * mSize);
    void* ret = &s->pMemBuffer[s->size];
    s->size += aligned;
    s->pLastAlloc = ret;

    assert(s->size < s->cap && "Out of memory");

    return ret;
}

constexpr void*
FixedRealloc(FixedAllocator* s, void* p, u64 mCount, u64 mSize)
{
    void* ret = nullptr;
    u64 aligned = align8(mCount * mSize);

    if (p == s->pLastAlloc)
    {
        s->size -= (u8*)&s->pMemBuffer[s->size] - (u8*)p;
        s->size += aligned;

        return p;
    }
    else
    {
        ret = &s->pMemBuffer[s->size];
        s->pLastAlloc = ret;
        u64 nBytesUntilEndOfBlock = s->cap - s->size;
        u64 nBytesToCopy = utils::min(aligned, nBytesUntilEndOfBlock);
        memcpy(ret, p, nBytesToCopy);
        s->size += aligned;
    }

    return ret;
}

constexpr void
FixedFree([[maybe_unused]] FixedAllocator* s, [[maybe_unused]] void* p)
{
    //
}

constexpr void
FixedFreeAll([[maybe_unused]] FixedAllocator* s)
{
    //
}

constexpr void
FixedReset(FixedAllocator* s)
{
    s->size = 0;
}

inline const AllocatorInterface inl_FixedAllocatorVTable {
    .alloc = decltype(AllocatorInterface::alloc)(FixedAlloc),
    .realloc = decltype(AllocatorInterface::realloc)(FixedRealloc),
    .free = decltype(AllocatorInterface::free)(FixedFree),
    .freeAll = decltype(AllocatorInterface::freeAll)(FixedFreeAll),
};

constexpr FixedAllocator::FixedAllocator(void* pMemory, u64 capacity)
    : base{.pVTable = &inl_FixedAllocatorVTable}, pMemBuffer((u8*)pMemory), cap(capacity) {}

} /* namespace adt */
