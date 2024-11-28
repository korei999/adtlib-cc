#pragma once

#include "Allocator.hh"
#include "utils.hh"

#include <cassert>
#include <cstring>

namespace adt
{

struct FixedAllocator;

constexpr void* FixedAlloc(FixedAllocator* s, u64 mCount, u64 mSize);
constexpr void* FixedZalloc(FixedAllocator* s, u64 mCount, u64 mSize);
constexpr void* FixedRealloc(FixedAllocator* s, void* p, u64 mCount, u64 mSize);
constexpr void FixedFree(FixedAllocator* s, void* p);
constexpr void FixedFreeAll(FixedAllocator* s);
constexpr void FixedReset(FixedAllocator* s);

struct FixedAllocator
{
    Allocator super {};
    u8* pMemBuffer = nullptr;
    u64 size = 0;
    u64 cap = 0;
    void* pLastAlloc = nullptr;

    constexpr FixedAllocator() = default;
    constexpr FixedAllocator(void* pMemory, u64 capacity);

    [[nodiscard]] constexpr void* alloc(u64 mCount, u64 mSize) { return FixedAlloc(this, mCount, mSize); }
    [[nodiscard]] constexpr void* zalloc(u64 mCount, u64 mSize) { return FixedZalloc(this, mCount, mSize); }
    [[nodiscard]] constexpr void* realloc(void* ptr, u64 mCount, u64 mSize) { return FixedRealloc(this, ptr, mCount, mSize); }
    constexpr void free(void* ptr) { FixedFree(this, ptr); }
    constexpr void freeAll() { FixedFreeAll(this); }
    constexpr void reset() { FixedReset(this); }
};

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
FixedZalloc(FixedAllocator* s, u64 mCount, u64 mSize)
{
    auto* p = FixedAlloc(s, mCount, mSize);
    memset(p, 0, mCount * mSize);
    return p;
}

constexpr void*
FixedRealloc(FixedAllocator* s, void* p, u64 mCount, u64 mSize)
{
    if (!p) return FixedAlloc(s, mCount, mSize);

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

inline const AllocatorVTable inl_FixedAllocatorVTable {
    .alloc = decltype(AllocatorVTable::alloc)(+[](FixedAllocator* s, u64 mCount, u64 mSize) { return s->alloc(mCount, mSize); }),
    .zalloc = decltype(AllocatorVTable::zalloc)(+[](FixedAllocator* s, u64 mCount, u64 mSize) { return s->zalloc(mCount, mSize); }),
    .realloc = decltype(AllocatorVTable::realloc)(+[](FixedAllocator* s, void* ptr, u64 mCount, u64 mSize) { return s->realloc(ptr, mCount, mSize); }),
    .free = decltype(AllocatorVTable::free)(+[](FixedAllocator* s, void* ptr) { s->free(ptr); }),
    .freeAll = decltype(AllocatorVTable::freeAll)(+[](FixedAllocator* s) { s->freeAll(); } ),
};

constexpr FixedAllocator::FixedAllocator(void* pMemory, u64 capacity)
    : super{.pVTable = &inl_FixedAllocatorVTable}, pMemBuffer((u8*)pMemory), cap(capacity) {}

} /* namespace adt */
