#pragma once

#include "IAllocator.hh"
#include "utils.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>

#if defined ADT_DBG_MEMORY
    #include <cstdio>
#endif

namespace adt
{

struct ArenaBlock
{
    ArenaBlock* pNext {};
    u64 size {}; /* excluding sizeof(ArenaBlock) */
    u64 nBytesOccupied {};
    u8* pLastAlloc {};
    u64 lastAllocSize {};
    u8 pMem[];
};

/* fast region based allocator, only freeAll() free's memory, free() does nothing */
struct Arena : IAllocator
{
    u64 defaultCapacity {};
    ArenaBlock* pBlocks {};

    virtual inline void* alloc(u64 mCount, u64 mSize) final;
    virtual inline void* zalloc(u64 mCount, u64 mSize) final;
    virtual inline void* realloc(void* ptr, u64 mCount, u64 mSize) final;
    virtual inline void free(void* ptr) final;
    virtual inline void freeAll() final;

    Arena() = default;
    Arena(u64 capacity);
};

[[nodiscard]] inline ArenaBlock*
_ArenaFindBlockFromPtr(Arena* s, u8* ptr)
{
    auto* it = s->pBlocks;
    while (it)
    {
        if (ptr >= it->pMem && ptr < &it->pMem[it->size])
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
        if (size < it->size - it->nBytesOccupied)
            return it;

        it = it->pNext;
    }

    return nullptr;
}

[[nodiscard]] inline ArenaBlock*
_ArenaAllocBlock(u64 size)
{
    ArenaBlock* pBlock = (ArenaBlock*)::calloc(1, size + sizeof(ArenaBlock));
    pBlock->size = size;
    pBlock->pLastAlloc = pBlock->pMem;

    return pBlock;
}

[[nodiscard]] inline ArenaBlock*
_ArenaPrependBlock(Arena* s, u64 size)
{
    auto* pNew = _ArenaAllocBlock(size);
    pNew->pNext = s->pBlocks;
    s->pBlocks = pNew;

    return pNew;
}

inline void*
Arena::alloc(u64 mCount, u64 mSize)
{
    auto* s = this;

    assert(s != nullptr && "[Arena]: nullptr self passed");

    u64 realSize = align8(mCount * mSize);
    auto* pBlock = _ArenaFindFittingBlock(s, realSize);

#if defined ADT_DBG_MEMORY
    if (s->defaultCapacity <= realSize)
        fprintf(stderr, "[Arena]: allocating more than defaultCapacity (%llu, %llu)\n", s->defaultCapacity, realSize);
#endif

    if (!pBlock) pBlock = _ArenaPrependBlock(s, utils::max(s->defaultCapacity, realSize*2));

    auto* pRet = pBlock->pMem + pBlock->nBytesOccupied;
    assert(pRet == pBlock->pLastAlloc + pBlock->lastAllocSize);

    pBlock->nBytesOccupied += realSize;
    pBlock->pLastAlloc = pRet;
    pBlock->lastAllocSize = realSize;

    return pRet;
}

inline void*
Arena::zalloc(u64 mCount, u64 mSize)
{
    auto* s = this;

    assert(s != nullptr && "[Arena]: nullptr self passed");

    auto* p = s->alloc(mCount, mSize);
    memset(p, 0, align8(mCount * mSize));
    return p;
}

inline void*
Arena::realloc(void* ptr, u64 mCount, u64 mSize)
{
    auto* s = this;

    assert(s != nullptr && "[Arena]: nullptr self passed");

    if (!ptr) return alloc(mCount, mSize);

    u64 requested = mSize * mCount;
    u64 realSize = align8(requested);
    auto* pBlock = _ArenaFindBlockFromPtr(s, (u8*)ptr);

    assert(pBlock && "[Arena]: pointer doesn't belong to this arena");

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
        auto* pRet = alloc(mCount, mSize);
        u64 nBytesUntilEndOfBlock = &pBlock->pMem[pBlock->size] - (u8*)ptr;
        u64 nBytesToCopy = utils::min(requested, nBytesUntilEndOfBlock); /* out of range memcpy */
        nBytesToCopy = utils::min(nBytesToCopy, u64((u8*)pRet - (u8*)ptr)); /* overlap memcpy */
        memcpy(pRet, ptr, nBytesToCopy);

        return pRet;
    }
}

inline void
Arena::free(void*)
{
    /* noop */
}

inline void
Arena::freeAll()
{
    assert(s != nullptr && "[Arena]: nullptr self passed");

    auto* it = this->pBlocks;
    while (it)
    {
        auto* next = it->pNext;
        ::free(it);
        it = next;
    }
    this->pBlocks = nullptr;
}

inline void
ArenaReset(Arena* s)
{
    assert(s != nullptr && "[Arena]: nullptr self passed");

    auto* it = s->pBlocks;
    while (it)
    {
        it->nBytesOccupied = 0;
        it->lastAllocSize = 0;
        it->pLastAlloc = it->pMem;

        it = it->pNext;
    }
}

inline Arena::Arena(u64 capacity)
    : defaultCapacity(align8(capacity)),
      pBlocks(_ArenaAllocBlock(this->defaultCapacity)) {}

} /* namespace adt */
