#pragma once

#include "Allocator.hh"
#include "utils.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace adt
{

struct ArenaBlock
{
    ArenaBlock* pNext {};
    u64 size {}; /* excluding sizeof(Arena2Block) */
    u64 nBytesOccupied {};
    u8* pLastAlloc {};
    u64 lastAllocSize {};
    u8 pMem[];
};

struct Arena
{
    Allocator base {};
    u64 defaultCapacity = 0;
    ArenaBlock* pBlocks {};

    Arena() = default;
    Arena(u64 capacity);
};

[[nodiscard]] inline void* ArenaAlloc(Arena* s, u64 mCount, u64 mSize);
[[nodiscard]] inline void* ArenaRealloc(Arena* s, void* ptr, u64 mCount, u64 mSize);
inline void ArenaFree(Arena* s, void* ptr);
inline void ArenaFreeAll(Arena* s);
inline void ArenaReset(Arena* s);

[[nodiscard]] inline void* alloc(Arena* s, u64 mCount, u64 mSize) { return ArenaAlloc(s, mCount, mSize); }
[[nodiscard]] inline void* realloc(Arena* s, void* ptr, u64 mCount, u64 mSize) { return ArenaRealloc(s, ptr, mCount, mSize); }
inline void free(Arena* s, void* ptr) { return ArenaFree(s, ptr); }
inline void freeAll(Arena* s) { return ArenaFreeAll(s); }

[[nodiscard]] inline ArenaBlock*
_ArenaFindBlockFromPtr(Arena* s, u8* ptr)
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

[[nodiscard]] inline ArenaBlock*
_ArenaFindFittingBlock(Arena* s, u64 size)
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

[[nodiscard]] inline ArenaBlock*
_ArenaAllocBlock(Arena* s, u64 size)
{
    ArenaBlock* pBlock = (ArenaBlock*)::calloc(1, size + sizeof(ArenaBlock));
    pBlock->size = size;
    pBlock->pLastAlloc = pBlock->pMem;

    return pBlock;
}

[[nodiscard]] inline ArenaBlock*
_ArenaAppendBlock(Arena* s, u64 size)
{
    assert(s->pBlocks && "[Arena2]: wasn't initialized");

    auto* it = s->pBlocks;
    while (it->pNext) it = it->pNext;
    it->pNext = _ArenaAllocBlock(s, size);

    return it->pNext;
}

inline void*
ArenaAlloc(Arena* s, u64 mCount, u64 mSize)
{
    u64 realSize = align8(mCount * mSize);
    auto* pBlock = _ArenaFindFittingBlock(s, realSize);
    if (!pBlock) pBlock = _ArenaAppendBlock(s, utils::max(s->defaultCapacity, realSize*2));

    pBlock->nBytesOccupied += realSize;
    auto* pRet = pBlock->pLastAlloc + pBlock->lastAllocSize;

    pBlock->lastAllocSize = realSize;
    pBlock->pLastAlloc = pRet;

    return pRet;
}

inline void*
ArenaRealloc(Arena* s, void* ptr, u64 mCount, u64 mSize)
{
    assert(ptr != nullptr && "[Arena2]: passing nullptr to realloc");

    u64 requested = mSize * mCount;
    u64 realSize = align8(requested);
    auto* pBlock = _ArenaFindBlockFromPtr(s, (u8*)ptr);

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
        auto* pRet = ArenaAlloc(s, mCount, mSize);
        u64 nBytesUntilEndOfBlock = pBlock->size - u64((u8*)ptr - pBlock->pMem);
        u64 nBytesToCopy = utils::min(requested, nBytesUntilEndOfBlock); /* out of range memcpy */
        nBytesToCopy = utils::min(nBytesToCopy, u64((u8*)pRet - (u8*)ptr)); /* overlap memcpy */
        memcpy(pRet, ptr, nBytesToCopy);
        return pRet;
    }
}

inline void
ArenaFree([[maybe_unused]] Arena* s, [[maybe_unused]] void* ptr)
{
    //
}

inline void
ArenaFreeAll(Arena* s)
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
ArenaReset(Arena* s)
{
    auto* it = s->pBlocks;
    while (it)
    {
        it->nBytesOccupied = 0;
        it->pLastAlloc = it->pMem;

        it = it->pNext;
    }
}

inline const AllocatorInterface inl_Arena2VTable {
    .alloc = decltype(AllocatorInterface::alloc)(ArenaAlloc),
    .realloc = decltype(AllocatorInterface::realloc)(ArenaRealloc),
    .free = decltype(AllocatorInterface::free)(ArenaFree),
    .freeAll = decltype(AllocatorInterface::freeAll)(ArenaFreeAll),
};

inline Arena::Arena(u64 capacity)
    : base(&inl_Arena2VTable),
      defaultCapacity(align8(capacity)),
      pBlocks(_ArenaAllocBlock(this, this->defaultCapacity)) {}

} /* namespace adt */
