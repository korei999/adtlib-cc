#pragma once

#include "Allocator.hh"
#include "utils.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace adt
{

struct Arena2Block
{
    Arena2Block* pNext {};
    u64 size {}; /* excluding sizeof(Arena2Block) */
    u64 nBytesOccupied {};
    u8* pLastAlloc {};
    u64 lastAllocSize {};
    u8 pMem[];
};

struct Arena2
{
    Allocator base {};
    u64 defaultCapacity = 0;
    Arena2Block* pBlocks {};

    Arena2() = default;
    Arena2(u64 capacity);
};

[[nodiscard]] inline void* Arena2Alloc(Arena2* s, u64 mCount, u64 mSize);
[[nodiscard]] inline void* Arena2Realloc(Arena2* s, void* ptr, u64 mCount, u64 mSize);
inline void Arena2Free(Arena2* s, void* ptr);
inline void Arena2FreeAll(Arena2* s);
inline void Arena2Reset(Arena2* s);

[[nodiscard]] inline void* alloc(Arena2* s, u64 mCount, u64 mSize) { return Arena2Alloc(s, mCount, mSize); }
[[nodiscard]] inline void* realloc(Arena2* s, void* ptr, u64 mCount, u64 mSize) { return Arena2Realloc(s, ptr, mCount, mSize); }
inline void free(Arena2* s, void* ptr) { return Arena2Free(s, ptr); }
inline void freeAll(Arena2* s) { return Arena2FreeAll(s); }

[[nodiscard]] inline Arena2Block*
_Arena2FindBlockFromPtr(Arena2* s, u8* ptr)
{
    auto* it = s->pBlocks;
    while (it)
    {
        if (ptr >= it->pMem && ptr < (it->pMem + it->size))
            return it;

        it = it->pNext;
    }

    return nullptr;
}

[[nodiscard]] inline Arena2Block*
_Arena2FindFittingBlock(Arena2* s, u64 size)
{
    auto* it = s->pBlocks;
    while (it)
    {
        if (it->size - it->nBytesOccupied > size)
            return it;

        it = it->pNext;
    }

    return nullptr;
}

[[nodiscard]] inline Arena2Block*
_Arena2AllocBlock(Arena2* s, u64 size)
{
    Arena2Block* pBlock = (Arena2Block*)::calloc(1, size + sizeof(Arena2Block));
    pBlock->size = size;
    pBlock->pLastAlloc = pBlock->pMem;

    return pBlock;
}

[[nodiscard]] inline Arena2Block*
_Arena2AppendBlock(Arena2* s, u64 size)
{
    assert(s->pBlocks && "[Arena2]: wasn't initialized");

    auto* it = s->pBlocks;
    while (it->pNext) it = it->pNext;
    it->pNext = _Arena2AllocBlock(s, size);

    return it->pNext;
}

inline void*
Arena2Alloc(Arena2* s, u64 mCount, u64 mSize)
{
    u64 realSize = align8(mCount * mSize);
    auto* pBlock = _Arena2FindFittingBlock(s, realSize);
    if (!pBlock) pBlock = _Arena2AppendBlock(s, utils::max(s->defaultCapacity, realSize*2));

    pBlock->nBytesOccupied += realSize;
    auto* pRet = pBlock->pLastAlloc + pBlock->lastAllocSize;

    pBlock->lastAllocSize = realSize;
    pBlock->pLastAlloc = pRet;

    return pRet;
}

inline void*
Arena2Realloc(Arena2* s, void* ptr, u64 mCount, u64 mSize)
{
    assert(ptr != nullptr && "[Arena2]: passing nullptr to realloc");

    u64 requested = mSize * mCount;
    u64 realSize = align8(requested);
    auto* pBlock = _Arena2FindBlockFromPtr(s, (u8*)ptr);

    assert(pBlock && "[Arena2]: pointer doesn't belong to this arena");

    if (ptr == pBlock->pLastAlloc &&
        pBlock->pLastAlloc + realSize < pBlock->pMem + pBlock->size) /* bump case */
    {
        pBlock->nBytesOccupied -= pBlock->lastAllocSize;
        pBlock->nBytesOccupied += realSize;
        pBlock->lastAllocSize = realSize;
        return ptr;
    }
    else
    {
        auto* pRet = Arena2Alloc(s, mCount, mSize);
        u64 nBytesUntilEndOfBlock = pBlock->size - u64((u8*)ptr - pBlock->pMem);
        u64 nBytesToCopy = utils::min(requested, nBytesUntilEndOfBlock); /* out of range memcpy */
        nBytesToCopy = utils::min(nBytesToCopy, u64((u8*)pRet - (u8*)ptr)); /* overlap memcpy */
        memcpy(pRet, ptr, nBytesToCopy);
        return pRet;
    }
}

inline void
Arena2Free([[maybe_unused]] Arena2* s, [[maybe_unused]] void* ptr)
{
    //
}

inline void
Arena2FreeAll(Arena2* s)
{
    auto* it = s->pBlocks;
    while (it)
    {
        auto* next = it->pNext;
        ::free(it);
        it = next;
    }
    s->pBlocks = nullptr;
}

inline void
Arena2Reset(Arena2* s)
{
    auto* it = s->pBlocks;
    while (it)
    {
        it->nBytesOccupied = 0;
        it->pLastAlloc = it->pMem;

        it = it->pNext;
    }
}

inline const AllocatorInterface inl_arena2VTable {
    .alloc = decltype(AllocatorInterface::alloc)(Arena2Alloc),
    .realloc = decltype(AllocatorInterface::realloc)(Arena2Realloc),
    .free = decltype(AllocatorInterface::free)(Arena2Free),
    .freeAll = decltype(AllocatorInterface::freeAll)(Arena2FreeAll),
};

inline Arena2::Arena2(u64 capacity)
    : base(&inl_arena2VTable),
      defaultCapacity(align8(capacity)),
      pBlocks(_Arena2AllocBlock(this, this->defaultCapacity)) {}

} /* namespace adt */
