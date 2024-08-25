#pragma once

#include "RBTree.hh"
#include "logs.hh"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

namespace adt
{

struct FreeListNode
{
    u32 size = 0;
    bool bFreed;
    u8 pData[];
};

template<>
constexpr s64
utils::compare(const FreeListNode& l, const FreeListNode& r)
{
    return l.size - r.size;
}

struct FreeListBlock
{
    u64 size = 0;
    /*u64 used = 0;*/
    FreeListBlock* pNext;
    RBNode<FreeListNode> firstNode;
};

struct FreeList
{
    Allocator base {};
    FreeListBlock* pBlocks = nullptr;
    RBTree<FreeListNode> tree {};
    s64 used = 0;

    FreeList() = default;
    FreeList(u64 prealloc);
};

inline void*
FreeListAlloc(FreeList* s, u64 mCount, u64 mSize)
{
    u64 size = align8(mCount * mSize);
    u64 aligned = (size) + sizeof(RBNode<FreeListNode>);

    RBNode<FreeListNode>* node = s->tree.pRoot, * pLastFit = nullptr;
    while (node)
    {
        if (aligned <= node->data.size)
        {
            pLastFit = node;
            node = node->left;
        }
        else node = node->right;
    }

    u64 savedSize = pLastFit->data.size;
    /*COUT("size: %ld, aligned: %ld, savedSize: %ld\n", size, aligned, savedSize);*/

    if (!pLastFit)
    {
        assert(pLastFit && "No free node?");
        return nullptr;
    }

    RBRemove(&s->tree, pLastFit);

    auto* next = (RBNode<FreeListNode>*)((u8*)pLastFit + aligned);
    /*COUT("next.size: %ld, aligned: %ld\n", next->data.size, aligned);*/

    /* NOTE: implement coalescence */
    if (next->data.bFreed == true || next->data.size == 0)
    {
        next->data.size = savedSize - aligned;
        pLastFit->data.size = aligned;
        RBInsert(&s->tree, next, true);
    }

    /*f.b->left = f.b->right = f.b->parent = {};*/

    s->used += pLastFit->data.size;

    /*COUT("alloc: size: %ld, used: %ld\n", next->data.size, s->used);*/
    /*COUT("allocating: size: %ld\n", f.b->data.size);*/
    pLastFit->data.bFreed = false;
    return pLastFit->data.pData;
}

inline void*
FreeListRealloc(FreeList* s, void* p, u64 mCount, u64 mSize)
{
    auto* r = FreeListAlloc(s, mCount, mSize);
    memcpy(r, p, mCount * mSize);
    return r;
}

inline void
FreeListFree(FreeList* s, void* p)
{
    auto* node = (RBNode<FreeListNode>*)((u8*)p - sizeof(RBNode<FreeListNode>));
    node->data.bFreed = true;
    /*u64 size = node->data.size;*/
    /*COUT("free:  size: %ld, data: %d\n", size, ((RBNode<int>*)&node->data.pData)->data);*/

    /*node->left = node->right = node->parent = nullptr;*/
    RBInsert(&s->tree, node, true);
}

inline void
FreeListFreeAll(FreeList* s)
{
    ::free(s->pBlocks);
}

inline
FreeList::FreeList(u64 preallocBlock)
{
    static const Allocator::Interface vTable {
        .alloc = (decltype(Allocator::Interface::alloc))FreeListAlloc,
        .realloc = (decltype(Allocator::Interface::realloc))FreeListRealloc,
        .free = (decltype(Allocator::Interface::free))FreeListFree,
    };

    this->base = {&vTable};

    u64 aligned = align8(preallocBlock);

    this->pBlocks = (FreeListBlock*)::calloc(1, aligned + sizeof(FreeListBlock));
    this->pBlocks->size = aligned;
    this->pBlocks->pNext = nullptr;

    auto* firstNode = &this->pBlocks->firstNode;
    firstNode->data.size = aligned - sizeof(RBNode<FreeListNode>);
    firstNode->data.bFreed = true;
    RBInsert(&this->tree, firstNode, true);

    /*this->used = aligned + sizeof(FreeListBlock);*/
}

} /* namespace adt */
