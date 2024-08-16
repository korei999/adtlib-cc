#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "Allocator.hh"

#ifdef DEBUG
    #include "logs.hh"
#endif

#define ARENA_FIRST(A) ((A)->pBlocksHead)
#define ARENA_NEXT(AB) ((AB)->pNext)
#define ARENA_FOREACH(A, IT) for (ArenaBlock* IT = ARENA_FIRST(A); IT; IT = ARENA_NEXT(IT))
#define ARENA_FOREACH_SAFE(A, IT, TMP) for (ArenaBlock* IT = ARENA_FIRST(A), * TMP = nullptr; IT && ((TMP) = ARENA_NEXT(IT), true); (IT) = (TMP))
#define ARENA_GET_NODE_FROM_BLOCK(PB) ((ArenaNode*)((u8*)(PB) + offsetof(ArenaBlock, pData)))
#define ARENA_GET_NODE_FROM_DATA(PD) ((ArenaNode*)((u8*)(PD) - offsetof(ArenaNode, pData)))

namespace adt
{

struct ArenaNode;

struct ArenaBlock
{
    ArenaBlock* pNext = nullptr;
    u64 size = 0;
    ArenaNode* pLast = nullptr;
    u8 pData[]; /* flexible array member */
};

struct ArenaNode
{
    ArenaNode* pNext = nullptr;
    u8 pData[];
};

struct ArenaAllocator
{
    Allocator base {};
    ArenaBlock* pBlocksHead = nullptr;
    ArenaBlock* pLastBlockAllocation = nullptr;

    ArenaAllocator() = default;
    ArenaAllocator(u32 blockCap);
};

inline ArenaBlock*
ArenaAllocatorNewBlock(ArenaAllocator* s, u64 size)
{
    ArenaBlock** ppLastBlock = &s->pBlocksHead;
    while (*ppLastBlock) ppLastBlock = &((*ppLastBlock)->pNext);

    u64 addedSize = size + sizeof(ArenaBlock);

    *ppLastBlock = (ArenaBlock*)(calloc(1, addedSize));

    auto* pBlock = (ArenaBlock*)*ppLastBlock;
    pBlock->size = size;
    auto* pNode = ARENA_GET_NODE_FROM_BLOCK(*ppLastBlock);
    pNode->pNext = pNode; /* don't bump the very first node on `alloc()` */
    pBlock->pLast = pNode;
    s->pLastBlockAllocation = *ppLastBlock;

    return *ppLastBlock;
}

inline void*
ArenaAllocatorAlloc(ArenaAllocator* s, u64 mCount, u64 mSize)
{
    u64 requested = mCount * mSize;
    u64 aligned = align8(requested + sizeof(ArenaNode));

    /* TODO: find block that can fit */
    ArenaBlock* pFreeBlock = s->pBlocksHead;

    while (aligned >= pFreeBlock->size)
    {
#ifdef DEBUG
        LOG_WARN("requested size > than one block\n"
                 "aligned: %zu, blockSize: %zu, requested: %zu\n", aligned, pFreeBlock->size, requested);
#endif

        pFreeBlock = pFreeBlock->pNext;
        if (!pFreeBlock) pFreeBlock =  ArenaAllocatorNewBlock(s, aligned * 2); /* NOTE: trying to double too big of an array situation */
    }

repeat:
    /* skip pNext */
    ArenaNode* pNode = ARENA_GET_NODE_FROM_BLOCK(pFreeBlock);
    ArenaNode* pNextNode = pFreeBlock->pLast->pNext;
    u64 nextAligned = ((u8*)pNextNode + aligned) - (u8*)pNode;

    /* heap overflow */
    if (nextAligned >= pFreeBlock->size)
    {
        pFreeBlock = pFreeBlock->pNext;
        if (!pFreeBlock) pFreeBlock = ArenaAllocatorNewBlock(s, nextAligned);
        goto repeat;
    }

    pNextNode->pNext = (ArenaNode*)((u8*)pNextNode + aligned);
    pFreeBlock->pLast = pNextNode;

    return &pNextNode->pData;
}

inline void*
ArenaAllocatorRealloc(ArenaAllocator* s, void* p, u64 mCount, u64 mSize)
{
    ArenaNode* pNode = ARENA_GET_NODE_FROM_DATA(p);
    ArenaBlock* pBlock = nullptr;

    /* figure out which block this node belongs to */
    ARENA_FOREACH(s, pB)
        if ((u8*)p > (u8*)pB && ((u8*)pB + pB->size) > (u8*)p)
            pBlock = pB;

    assert(pBlock != nullptr && "block not found, bad pointer");

    auto aligned = align8(mCount * mSize);
    u64 nextAligned = ((u8*)pNode + aligned) - (u8*)ARENA_GET_NODE_FROM_BLOCK(pBlock);

    if (pNode == pBlock->pLast && nextAligned < pBlock->size)
    {
        /* NOTE: + sizeof(ArenaNode) is necessary */
        pNode->pNext = (ArenaNode*)((u8*)pNode + aligned + sizeof(ArenaNode));

        return p;
    }
    else
    {
        void* pR = ArenaAllocatorAlloc(s, mCount, mSize);
        memcpy(pR, p, ((u8*)pNode->pNext - (u8*)pNode));

        return pR;
    }
}

inline void
ArenaAllocatorFree([[maybe_unused]] ArenaAllocator* s, [[maybe_unused]] void* p)
{
    /* no individual frees */
}

inline void
ArenaAllocatorReset(ArenaAllocator* s)
{
    ARENA_FOREACH(s, pB)
    {
        ArenaNode* pNode = ARENA_GET_NODE_FROM_BLOCK(pB);
        pB->pLast = pNode;
        pNode->pNext = pNode;
    }

    auto first = ARENA_FIRST(s);
    s->pLastBlockAllocation = first;
}

inline void
ArenaAllocatorFreeAll(ArenaAllocator* s)
{
    ARENA_FOREACH_SAFE(s, pB, tmp)
        ::free(pB);
}

inline const Allocator::Interface __ArenaAllocatorVTable {
    .alloc = decltype(Allocator::Interface::alloc)(ArenaAllocatorAlloc),
    .realloc = decltype(Allocator::Interface::realloc)(ArenaAllocatorRealloc),
    .free = decltype(Allocator::Interface::free)(ArenaAllocatorFree)
};

inline 
ArenaAllocator::ArenaAllocator(u32 blockCap)
    : base {&__ArenaAllocatorVTable}
{
    ArenaAllocatorNewBlock(this, align8(blockCap + sizeof(ArenaNode)));
}

} /* namespace adt */
