/* `Adapted` from OpenBSD source */

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

#include "Allocator.hh"
#include "String.hh"
#include "compare.hh"

#include <stdio.h>

namespace adt
{

enum class RB_COL : u8 { BLACK, RED };
enum class RB_ORDER { PRE, IN, POST };

template<typename T>
struct RBNode
{
    RBNode* left;
    RBNode* right;
    RBNode* parent;
    enum RB_COL color;
    T data;
};

template<typename T>
struct RB
{
    Allocator* pAlloc = nullptr;
    RBNode<T>* pRoot = nullptr;

    RB() = default;
    RB(Allocator* pA) : pAlloc{pA} {}
};

template<typename T>
inline void
RBSetLinks(RBNode<T>* l, RBNode<T>* r)
{
    l->left = r->left;
    l->right = r->right;
    l->parent = r->parent;
    l->color = r->color;
}

template<typename T>
inline void
RBSet(RBNode<T>* elm, RBNode<T>* parent)
{
    elm->parent = parent;
    elm->left = elm->right = nullptr;
    elm->color = RB_COL::RED;
}

template<typename T>
inline void
RBSetBlackRed(RBNode<T>* black, RBNode<T>* red)
{
    black->color = RB_COL::BLACK;
    red->color = RB_COL::RED;
}

template<typename T>
inline RBNode<T>*
RBNodeAlloc(Allocator* pA, const T& data)
{
    auto* r = (RBNode<T>*)alloc(pA, 1, sizeof(RBNode<T>));
    r->data = data;
    return r;
}

template<typename T>
inline bool
RBEmpty(RB<T>* s)
{
    return s->pRoot;
}

template<typename T>
constexpr void
RBRotateLeft(RB<T>* s, RBNode<T>* elm)
{
    auto tmp = elm->right;
    if ((elm->right = tmp->left))
    {
        tmp->left->parent = elm;
    }
    if ((tmp->parent = elm->parent))
    {
        if (elm == elm->parent->left)
            elm->parent->left = tmp;
        else
            elm->parent->right = tmp;
    }
    else
        s->pRoot = tmp;
    tmp->left = elm;
    elm->parent = tmp;
}

template<typename T>
constexpr void
RBRotateRight(RB<T>* s, RBNode<T>* elm)
{
    auto tmp = elm->left;
    if ((elm->left = tmp->right))
    {
        tmp->right->parent = elm;
    }
    if ((tmp->parent = elm->parent))
    {
        if (elm == elm->parent->left)
            elm->parent->left = tmp;
        else
            elm->parent->right = tmp;
    }
    else
        s->pRoot = tmp;

    tmp->right = elm;
    elm->parent = tmp;
}

template<typename T>
inline void
RBInsertColor(RB<T>* s, RBNode<T>* elm)
{
    RBNode<T>* parent, * gparent, * tmp;
    while ((parent = elm->parent) && parent->color == RB_COL::RED)
    {
        gparent = parent->parent;
        if (parent == gparent->left)
        {
            tmp = gparent->right;
            if (tmp && tmp->color == RB_COL::RED)
            {
                tmp->color = RB_COL::BLACK;
                RBSetBlackRed(parent, gparent);
                elm = gparent;
                continue;
            }
            if (parent->right == elm)
            {
                RBRotateLeft(s, parent);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RBSetBlackRed(parent, gparent);
            RBRotateRight(s, gparent);
        }
        else
        {
            tmp = gparent->left;
            if (tmp && tmp->color == RB_COL::RED)
            {
                tmp->color = RB_COL::BLACK;
                RBSetBlackRed(parent, gparent);
                elm = gparent;
                continue;
            }
            if (parent->left == elm)
            {
                RBRotateRight(s, parent);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RBSetBlackRed(parent, gparent);
            RBRotateLeft(s, gparent);
        }
    }
    s->pRoot->color = RB_COL::BLACK;
}

template<typename T>
void RBRemoveColor(RB<T>* s, RBNode<T>* parent, RBNode<T>* elm)                 
{
    RBNode<T>* tmp;
    while ((elm == nullptr || elm->color == RB_COL::BLACK) && elm != s->pRoot)
    {
        if (parent->left == elm)
        {
            tmp = parent->right;
            if (tmp->color == RB_COL::RED)
            {
                RBSetBlackRed(tmp, parent);
                RBRotateLeft(s, parent);
                tmp = parent->right;
            }
            if ((tmp->left == nullptr || tmp->left->color == RB_COL::BLACK) &&
                (tmp->right == nullptr || tmp->right->color == RB_COL::BLACK))
            {
                tmp->color = RB_COL::RED;
                elm = parent;
                parent = elm->parent;
            }
            else
            {
                if (tmp->right == nullptr || tmp->right->color == RB_COL::BLACK)
                {
                    RBNode<T>* oleft;
                    if ((oleft = tmp->left))
                        oleft->color = RB_COL::BLACK;
                    tmp->color = RB_COL::RED;
                    RBRotateRight(s, tmp);
                    tmp = parent->right;
                }
                tmp->color = parent->color;
                parent->color = RB_COL::BLACK;
                if (tmp->right)
                    tmp->right->color = RB_COL::BLACK;
                RBRotateLeft(s, parent);
                elm = s->pRoot;
                break;
            }
        }
        else
        {
            tmp = parent->left;
            if (tmp->color == RB_COL::RED)
            {
                RBSetBlackRed(tmp, parent);
                RBRotateRight(s, parent);
                tmp = parent->left;
            }
            if ((tmp->left == nullptr || tmp->left->color == RB_COL::BLACK) &&
                (tmp->right == nullptr || tmp->right->color == RB_COL::BLACK))
            {
                tmp->color = RB_COL::RED;
                elm = parent;
                parent = elm->parent;
            }
            else
            {
                if (tmp->left == nullptr || tmp->left->color == RB_COL::BLACK)
                {
                    RBNode<T>* oright;
                    if ((oright = tmp->right))
                        oright->color = RB_COL::BLACK;
                    tmp->color = RB_COL::RED;
                    RBRotateLeft(s, tmp);
                    tmp = parent->left;
                }
                tmp->color = parent->color;
                parent->color = RB_COL::BLACK;
                if (tmp->left)
                    tmp->left->color = RB_COL::BLACK;
                RBRotateRight(s, parent);
                elm = s->pRoot;
                break;
            }
        }
    }
    if (elm)
        elm->color = RB_COL::BLACK;
}

template<typename T>
inline RBNode<T>*
RBRemove(RB<T>* s, RBNode<T>* elm)                       
{
    RBNode<T>* child, * parent, * old = elm;
    enum RB_COL color;
    if (elm->left == nullptr)
        child = elm->right;
    else if (elm->right == nullptr)
        child = elm->left;
    else
    {
        RBNode<T>* left;
        elm = elm->right;
        while ((left = elm->left))
            elm = left;
        child = elm->right;
        parent = elm->parent;
        color = elm->color;
        if (child)
            child->parent = parent;
        if (parent)
        {
            if (parent->left == elm)
                parent->left = child;
            else
                parent->right = child;
        }
        else
            s->pRoot = child;
        if (elm->parent == old)
            parent = elm;

        RBSetLinks(elm, old);

        if (old->parent)
        {
            if (old->parent->left == old)
                old->parent->left = elm;
            else
                old->parent->right = elm;
        }
        else
            s->pRoot = elm;
        old->left->parent = elm;
        if (old->right)
            old->right->parent = elm;
        goto color;
    }
    parent = elm->parent;
    color = elm->color;
    if (child)
        child->parent = parent;
    if (parent)
    {
        if (parent->left == elm)
            parent->left = child;
        else
            parent->right = child;
    }
    else
        s->pRoot = child;
color:
    if (color == RB_COL::BLACK)
        RBRemoveColor(s, parent, child);
    return (old);
}

/* create RBNode outside then insert */
template<typename T>
inline struct RBNode<T>*
RBInsert(RB<T>* s, RBNode<T>* elm)
{
    RBNode<T>* tmp;
    RBNode<T>* parent = nullptr;
    s64 comp = 0;
    tmp = s->pRoot;
    while (tmp)
    {
        parent = tmp;
        comp = compare(elm->data, parent->data);
        if (comp < 0)
            tmp = tmp->left;
        else if (comp > 0)
            tmp = tmp->right;
        else
            return tmp;
    }

    RBSet(elm, parent);

    if (parent != nullptr)
    {
        if (comp < 0)
            parent->left = elm;
        else
            parent->right = elm;
    }
    else
        s->pRoot = elm;
    RBInsertColor(s, elm);

    return elm;
}

template<typename T>
inline struct RBNode<T>*
RBInsert(RB<T>* s, const T& data)
{
    RBNode<T>* pNew = RBNodeAlloc(s->pAlloc, data);
    return RBInsert(s, pNew);
}

/* early return if pfn returns true */
template<typename T>
inline void
RBTraverse(
    RBNode<T>* p,
    bool (*pfn)(RBNode<T>*, void*),
    void* pUserData,
    enum RB_ORDER order
)
{
    if (p)
    {
        switch (order)
        {
            case RB_ORDER::PRE:
                if (pfn(p, pUserData)) return;
                RBTraverse(p->left, pfn, pUserData, order);
                RBTraverse(p->right, pfn, pUserData, order);
                break;

            case RB_ORDER::IN:
                RBTraverse(p->left, pfn, pUserData, order);
                if (pfn(p, pUserData)) return;
                RBTraverse(p->right, pfn, pUserData, order);
                break;

            case RB_ORDER::POST:
                RBTraverse(p->left, pfn, pUserData, order);
                RBTraverse(p->right, pfn, pUserData, order);
                if (pfn(p, pUserData)) return;
                break;
        }
    }
}

template<typename T>
inline RBNode<T>*
RBSearch(RBNode<T>* p, const T& data)
{
    if (p)
    {
        if (data == p->data) return p;
        else if (data < p->data) return RBSearch(p->pLeft, data);
        else return RBSearch(p->pRight, data);
    } else return nullptr;
}

template<typename T>
inline short
RBDepth(RBNode<T>* p)
{
    if (p)
    {
        short l = RBDepth(p->left);
        short r = RBDepth(p->right);
        return 1 + max(l, r);
    } else return 0;
}

template<typename T>
inline void
RBPrintNodes(
    Allocator* pA,
    const RB<T>* s,
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
        fprintf(pF, "%.*s%s", sPrefix.size, sPrefix.pData, bLeft ? "├──" : "└──");
        pfnPrint(pNode, pFnData);

        String sCat = StringCat(pA, sPrefix, bLeft ? "│   " : "    ");

        RBPrintNodes(pA, s, pNode->left, pfnPrint, pFnData, pF, sCat, true);
        RBPrintNodes(pA, s, pNode->right, pfnPrint, pFnData, pF, sCat, false);
    }
}

} /* namespace adt */
