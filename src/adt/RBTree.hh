/* Borrowed from OpenBSD's red-black tree implementation. */

/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "IAllocator.hh"
#include "String.hh"
#include "utils.hh"

#include <cstdio>
#include <cassert>

#ifdef _WIN32
    #undef IN
#endif

namespace adt
{

enum class RB_COLOR : u8 { BLACK, RED };
enum class RB_ORDER : u8 { PRE, IN, POST };

constexpr u64 RB_COLOR_MASK = 1ULL;

template<typename T>
struct RBNode
{
    RBNode* left {};
    RBNode* right {};
    RBNode* parentPlusColor {}; /* NOTE: color is stored as the least significant bit */
    T data {};
};

template<typename T>
inline RBNode<T>*& RBLeft(RBNode<T>* s) { return s->left; }

template<typename T>
inline RBNode<T>* const& RBLeft(const RBNode<T>* s) { return s->left; }

template<typename T>
inline RBNode<T>*& RBRight(RBNode<T>* s) { return s->right; }

template<typename T>
inline RBNode<T>* const& RBRight(const RBNode<T>* s) { return s->right; }

template<typename T>
inline RBNode<T>* RBParent(RBNode<T>* s) { return (RBNode<T>*)((u64)s->parentPlusColor & ~RB_COLOR_MASK); }

template<typename T>
inline RB_COLOR RBColor(const RBNode<T>* s) { return (RB_COLOR)((u64)s->parentPlusColor & RB_COLOR_MASK); }

template<typename T>
inline void RBSetParent(RBNode<T>* s, RBNode<T>* par) { s->parentPlusColor = (RBNode<T>*)(((u64)par & ~RB_COLOR_MASK) | (u64)RBColor<T>(s)); }

template<typename T>
inline RB_COLOR RBSetColor(RBNode<T>* s, RB_COLOR eColor) { s->parentPlusColor = (RBNode<T>*)((u64)RBParent<T>(s) | (u64)eColor); return eColor; }

template<typename T>
inline RBNode<T>*& RBParCol(RBNode<T>* s) { return s->parentPlusColor; }

template<typename T>
inline RBNode<T>* const& RBParCol(const RBNode<T>* s) { return s->parentPlusColor; }

template<typename T>
struct RBTreeBase
{
    RBNode<T>* pRoot = nullptr;
    u64 size = 0;
};

template<typename T>
inline RBNode<T>* RBRoot(RBTreeBase<T>* s);

template<typename T>
inline RBNode<T>* RBNodeAlloc(IAllocator* pA, const T& data);

template<typename T>
inline bool RBEmpty(RBTreeBase<T>* s);

template<typename T>
inline RBNode<T>* RBRemove(RBTreeBase<T>* s, RBNode<T>* elm);

template<typename T>
inline void RBRemoveAndFree(RBTreeBase<T>* s, IAllocator* p, RBNode<T>* elm);

template<typename T>
inline RBNode<T>* RBInsert(RBTreeBase<T>* s, RBNode<T>* elm, bool bAllowDuplicates);

template<typename T>
inline RBNode<T>* RBInsert(RBTreeBase<T>* s, IAllocator* pA, const T& data, bool bAllowDuplicates);

template<typename T>
inline RBNode<T>*
RBTraverse(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>* pNode, void* pArg),
    void* pUserData,
    RB_ORDER order
);

template<typename T>
inline RBNode<T>* RBSearch(RBNode<T>* p, const T& data);

template<typename T>
inline int RBDepth(RBNode<T>* p);

template<typename T>
inline void
RBPrintNodes(
    IAllocator* pA,
    const RBTreeBase<T>* s,
    const RBNode<T>* pNode,
    void (*pfnPrint)(const RBNode<T>*, void*),
    void* pFnData,
    FILE* pF,
    const String sPrefix,
    bool bLeft
);

template<typename T> inline void _RBSetBlackRed(RBNode<T>* black, RBNode<T>* red);
template<typename T> inline void _RBSet(RBNode<T>* elm, RBNode<T>* parent);
template<typename T> inline void _RBSetLinks(RBNode<T>* l, RBNode<T>* r);
template<typename T> inline void _RBRotateLeft(RBTreeBase<T>* s, RBNode<T>* elm);
template<typename T> inline void _RBRotateRight(RBTreeBase<T>* s, RBNode<T>* elm);
template<typename T> inline void _RBInsertColor(RBTreeBase<T>* s, RBNode<T>* elm);
template<typename T> inline void _RBRemoveColor(RBTreeBase<T>* s, RBNode<T>* parent, RBNode<T>* elm);

template<typename T>
inline void
_RBSetLinks(RBNode<T>* l, RBNode<T>* r)
{
    RBLeft(l) = RBLeft(r);
    RBRight(l) = RBRight(r);
    RBParCol(l) = RBParCol(r);
}

template<typename T>
inline void
_RBSet(RBNode<T>* elm, RBNode<T>* parent)
{
    RBSetParent(elm, parent);
    RBLeft(elm) = RBRight(elm) = nullptr;
    RBSetColor(elm, RB_COLOR::RED);
}

template<typename T>
inline void
_RBSetBlackRed(RBNode<T>* black, RBNode<T>* red)
{
    RBSetColor(black, RB_COLOR::BLACK);
    RBSetColor(red, RB_COLOR::RED);
}

template<typename T>
inline RBNode<T>*
RBRoot(RBTreeBase<T>* s)
{
    return s->pRoot;
}

template<typename T>
inline RBNode<T>*
RBNodeAlloc(IAllocator* pA, const T& data)
{
    auto* r = (RBNode<T>*)pA->alloc(1, sizeof(RBNode<T>));
    r->data = data;
    return r;
}

template<typename T>
inline bool
RBEmpty(RBTreeBase<T>* s)
{
    return s->pRoot;
}

template<typename T>
inline void
_RBRotateLeft(RBTreeBase<T>* s, RBNode<T>* elm)
{
    auto tmp = RBRight(elm);
    if ((RBRight(elm) = RBLeft(tmp)))
    {
        RBSetParent(RBLeft(tmp), elm);
    }

    RBSetParent(tmp, RBParCol(elm));
    if (RBParent(tmp))
    {
        if (elm == RBLeft(RBParent(elm)))
            RBLeft(RBParent(elm)) = tmp;
        else RBRight(RBParent(elm)) = tmp;
    }
    else s->pRoot = tmp;

    RBLeft(tmp) = elm;
    RBSetParent(elm, tmp);
}

template<typename T>
inline void
_RBRotateRight(RBTreeBase<T>* s, RBNode<T>* elm)
{
    auto tmp = RBLeft(elm);
    if ((RBLeft(elm) = RBRight(tmp)))
    {
        RBSetParent(RBRight(tmp), elm);
    }

    RBSetParent(tmp, RBParCol(elm));
    if (RBParent(tmp))
    {
        if (elm == RBLeft(RBParent(elm)))
            RBLeft(RBParent(elm)) = tmp;
        else RBRight(RBParent(elm)) = tmp;
    }
    else s->pRoot = tmp;

    RBRight(tmp) = elm;
    RBSetParent(elm, tmp);
}

template<typename T>
inline void
_RBInsertColor(RBTreeBase<T>* s, RBNode<T>* elm)
{
    RBNode<T>* parent, * gparent, * tmp;
    while ((parent = RBParent(elm)) && RBColor(parent) == RB_COLOR::RED)
    {
        gparent = RBParent(parent);
        if (parent == RBLeft(gparent))
        {
            tmp = RBRight(gparent);
            if (tmp && RBColor(tmp) == RB_COLOR::RED)
            {
                RBSetColor(tmp, RB_COLOR::BLACK);
                _RBSetBlackRed(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RBRight(parent) == elm)
            {
                _RBRotateLeft(s, parent);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            _RBSetBlackRed(parent, gparent);
            _RBRotateRight(s, gparent);
        }
        else
        {
            tmp = RBLeft(gparent);
            if (tmp && RBColor(tmp) == RB_COLOR::RED)
            {
                RBSetColor(tmp, RB_COLOR::BLACK);
                _RBSetBlackRed(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RBLeft(parent) == elm)
            {
                _RBRotateRight(s, parent);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            _RBSetBlackRed(parent, gparent);
            _RBRotateLeft(s, gparent);
        }
    }
    RBSetColor(s->pRoot, RB_COLOR::BLACK);
}

template<typename T>
inline void
_RBRemoveColor(RBTreeBase<T>* s, RBNode<T>* parent, RBNode<T>* elm)
{
    RBNode<T>* tmp;
    while ((elm == nullptr || RBColor(elm) == RB_COLOR::BLACK) && elm != s->pRoot)
    {
        if (RBLeft(parent) == elm)
        {
            tmp = RBRight(parent);
            if (RBColor(tmp) == RB_COLOR::RED)
            {
                _RBSetBlackRed(tmp, parent);
                _RBRotateLeft(s, parent);
                tmp = RBRight(parent);
            }
            if ((RBLeft(tmp) == nullptr || RBColor(RBLeft(tmp)) == RB_COLOR::BLACK) &&
                (RBRight(tmp) == nullptr || RBColor(RBRight(tmp)) == RB_COLOR::BLACK))
            {
                RBSetColor(tmp, RB_COLOR::RED);
                elm = parent;
                parent = RBParent(elm);
            }
            else
            {
                if (RBRight(tmp) == nullptr || RBColor(RBRight(tmp)) == RB_COLOR::BLACK)
                {
                    RBNode<T>* oleft;
                    if ((oleft = RBLeft(tmp)))
                        RBSetColor(oleft, RB_COLOR::BLACK);

                    RBSetColor(tmp, RB_COLOR::RED);
                    _RBRotateRight(s, tmp);
                    tmp = RBRight(parent);
                }
                RBSetColor(tmp, RBColor(parent));
                RBSetColor(parent, RB_COLOR::BLACK);
                if (RBRight(tmp))
                    RBSetColor(RBRight(tmp), RB_COLOR::BLACK);
                _RBRotateLeft(s, parent);
                elm = s->pRoot;
                break;
            }
        }
        else
        {
            tmp = RBLeft(parent);
            if (RBColor(tmp) == RB_COLOR::RED)
            {
                _RBSetBlackRed(tmp, parent);
                _RBRotateRight(s, parent);
                tmp = RBLeft(parent);
            }
            if ((RBLeft(tmp) == nullptr || RBColor(RBLeft(tmp)) == RB_COLOR::BLACK) &&
                (RBRight(tmp) == nullptr || RBColor(RBRight(tmp)) == RB_COLOR::BLACK))
            {
                RBSetColor(tmp, RB_COLOR::RED);
                elm = parent;
                parent = RBParent(elm);
            }
            else
            {
                if (RBLeft(tmp) == nullptr || RBColor(RBLeft(tmp)) == RB_COLOR::BLACK)
                {
                    RBNode<T>* oright;
                    if ((oright = RBRight(tmp)))
                        RBSetColor(oright, RB_COLOR::BLACK);
                    RBSetColor(tmp, RB_COLOR::RED);
                    _RBRotateLeft(s, tmp);
                    tmp = RBLeft(parent);
                }
                RBSetColor(tmp, RBColor(parent));
                RBSetColor(parent, RB_COLOR::BLACK);
                if (RBLeft(tmp))
                    RBSetColor(RBLeft(tmp), RB_COLOR::BLACK);
                _RBRotateRight(s, parent);
                elm = s->pRoot;
                break;
            }
        }
    }
    if (elm) RBSetColor(elm, RB_COLOR::BLACK);
}

template<typename T>
inline RBNode<T>*
RBRemove(RBTreeBase<T>* s, RBNode<T>* elm)
{
    assert(s->size > 0 && "[RBTree]: empty");

    RBNode<T>* child, * parent, * old = elm;
    RB_COLOR color;
    if (RBLeft(elm) == nullptr)
        child = RBRight(elm);
    else if (RBRight(elm) == nullptr)
        child = RBLeft(elm);
    else
    {
        RBNode<T>* left;
        elm = RBRight(elm);
        while ((left = RBLeft(elm)))
            elm = left;
        child = RBRight(elm);
        parent = RBParent(elm);
        color = RBColor(elm);
        if (child)
            RBSetParent(child, parent);
        if (parent)
        {
            if (RBLeft(parent) == elm)
                RBLeft(parent) = child;
            else RBRight(parent) = child;
        }
        else s->pRoot = child;

        if (RBParent(elm) == old)
            parent = elm;

        _RBSetLinks(elm, old);

        if (RBParent(old))
        {
            if (RBLeft(RBParent(old)) == old)
                RBLeft(RBParent(old)) = elm;
            else
                RBRight(RBParent(old)) = elm;
        }
        else s->pRoot = elm;

        RBSetParent(RBLeft(old), elm);
        if (RBRight(old))
            RBSetParent(RBRight(old), elm);
        goto color;
    }
    parent = RBParent(elm);
    color = RBColor(elm);
    if (child)
        RBSetParent(child, parent);
    if (parent)
    {
        if (RBLeft(parent) == elm)
            RBLeft(parent) = child;
        else RBRight(parent) = child;
    }
    else
        s->pRoot = child;
color:
    if (color == RB_COLOR::BLACK)
        _RBRemoveColor(s, parent, child);

    --s->size;
    return (old);
}

template<typename T>
inline void
RBRemoveAndFree(RBTreeBase<T>* s, IAllocator* p, RBNode<T>* elm)
{
    free(p, RBRemove(s, elm));
}

/* create RBNode outside then insert */
template<typename T>
inline RBNode<T>*
RBInsert(RBTreeBase<T>* s, RBNode<T>* elm, bool bAllowDuplicates)
{
    RBNode<T>* parent = nullptr;
    RBNode<T>* tmp = s->pRoot;
    s64 comp = 0;
    while (tmp)
    {
        parent = tmp;
        comp = utils::compare(elm->data, parent->data);

        if (comp == 0)
        {
            /* left case */
            if (bAllowDuplicates) tmp = RBLeft(tmp);
            else return tmp;
        }
        else if (comp < 0) tmp = RBLeft(tmp);
        else tmp = RBRight(tmp);
    }

    _RBSet(elm, parent);

    if (parent != nullptr)
    {
        if (comp <= 0) RBLeft(parent) = elm;
        else RBRight(parent) = elm;
    }
    else s->pRoot = elm;

    _RBInsertColor(s, elm);
    ++s->size;
    return elm;
}

template<typename T>
inline RBNode<T>*
RBInsert(RBTreeBase<T>* s, IAllocator* pA, const T& data, bool bAllowDuplicates)
{
    RBNode<T>* pNew = RBNodeAlloc(pA, data);
    return RBInsert(s, pNew, bAllowDuplicates);
}

template<typename T>
inline RBNode<T>*
RBTraversePRE(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>* pNode, void* pArg),
    void* pUserData
)
{
    if (p)
    {
        if (pfn(p, pUserData)) return {p};
        RBTraversePRE(RBLeft(p), pfn, pUserData);
        RBTraversePRE(RBRight(p), pfn, pUserData);
    }

    return {};
}

template<typename T>
inline RBNode<T>*
RBTraverseIN(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>* pNode, void* pArg),
    void* pUserData
)
{
    if (p)
    {
        RBTraverseIN(RBLeft(p), pfn, pUserData);
        if (pfn(p, pUserData)) return {p};
        RBTraverseIN(RBRight(p), pfn, pUserData);
    }

    return {};
}

template<typename T>
inline RBNode<T>*
RBTraversePOST(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>* pNode, void* pArg),
    void* pUserData
)
{
    if (p)
    {
        RBTraversePOST(RBLeft(p), pfn, pUserData);
        RBTraversePOST(RBRight(p), pfn, pUserData);
        if (pfn(p, pUserData)) return {p};
    }

    return {};
}

/* early return if pfn returns true */
template<typename T>
inline RBNode<T>*
RBTraverse(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>* pNode, void* pArg),
    void* pUserData,
    RB_ORDER order
)
{
    switch (order)
    {
        case RB_ORDER::PRE:
        return RBTraversePRE(p, pfn, pUserData);

        case RB_ORDER::IN:
        return RBTraverseIN(p, pfn, pUserData);

        case RB_ORDER::POST:
        return RBTraversePOST(p, pfn, pUserData);
    }

    assert(false && "[RBTree]: incorrect RB_ORDER");
    return {};
}

template<typename T>
inline RBNode<T>*
RBSearch(RBNode<T>* p, const T& data)
{
    auto it = p;
    while (it)
    {
        s64 cmp = utils::compare(data, it->data);
        if (cmp == 0) return it;
        else if (cmp < 0) it = RBLeft(it);
        else it = RBRight(it);
    }

    return nullptr;
}

template<typename T>
inline int
RBDepth(RBNode<T>* p)
{
    if (p)
    {
        int l = RBDepth(RBLeft(p));
        int r = RBDepth(RBRight(p));
        return 1 + utils::max(l, r);
    } else return 0;
}

template<typename T>
inline void
RBPrintNodes(
    IAllocator* pA,
    const RBTreeBase<T>* s,
    const RBNode<T>* pNode,
    void (*pfnPrint)(const RBNode<T>*, void*),
    void* pFnData,
    FILE* pF,
    const String sPrefix,
    bool bLeft
)
{
    if (pNode)
    {
        fprintf(pF, "%.*s%s", sPrefix.size, sPrefix.pData, bLeft ? "|__" : "\\__");
        pfnPrint(pNode, pFnData);

        String sCat = StringCat(pA, sPrefix, bLeft ? "|   " : "    ");

        RBPrintNodes(pA, s, RBLeft(pNode), pfnPrint, pFnData, pF, sCat, true);
        RBPrintNodes(pA, s, RBRight(pNode), pfnPrint, pFnData, pF, sCat, false);

        pA->free(sCat.pData);
    }
}

template<typename T>
inline void
RBDestroy(RBTreeBase<T>* s, IAllocator* pAlloc)
{
    auto pfnFree = +[](RBNode<T>* p, void* data) -> bool {
        free((IAllocator*)data, p);

        return false;
    };

    RBTraverse(s->pRoot, pfnFree, pAlloc, RB_ORDER::POST);
}

template<typename T>
struct RBTree
{
    RBTreeBase<T> base {};
    IAllocator* pAlloc {};
    
    RBTree() = default;
    RBTree(IAllocator* p) : pAlloc(p) {}
};

template<typename T>
inline RBNode<T>* RBRoot(RBTree<T>* s) { return RBRoot<T>(&s->base); }

template<typename T>
inline bool RBEmpty(RBTree<T>* s) { return RBEmpty<T>(&s->base); }

template<typename T>
inline RBNode<T>* RBRemove(RBTree<T>* s, RBNode<T>* elm) { return RBRemove<T>(&s->base, elm); }

template<typename T>
inline RBNode<T>* RBRemove(RBTree<T>* s, const T& x) { return RBRemove<T>(&s->base, RBSearch<T>(s->base.pRoot, x)); }

template<typename T>
inline void RBRemoveAndFree(RBTree<T>* s, RBNode<T>* elm) { RBRemoveAndFree<T>(&s->base, s->pAlloc, elm); }

template<typename T>
inline void RBRemoveAndFree(RBTree<T>* s, const T& x) { RBRemoveAndFree(&s->base, s->pAlloc, RBSearch<T>(s->base.pRoot, x)); }

template<typename T>
inline RBNode<T>* RBInsert(RBTree<T>* s, RBNode<T>* elm, bool bAllowDuplicates) { return RBInsert<T>(&s->base, elm, bAllowDuplicates); }

template<typename T>
inline RBNode<T>* RBInsert(RBTree<T>* s, const T& data, bool bAllowDuplicates) { return RBInsert<T>(&s->base, s->pAlloc, data, bAllowDuplicates); }

template<typename T>
inline void RBDestroy(RBTree<T>* s) { RBDestroy<T>(&s->base, s->pAlloc); }

} /* namespace adt */
