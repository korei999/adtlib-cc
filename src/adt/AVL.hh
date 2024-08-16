#pragma once

#include "Allocator.hh"
#include "String.hh"
#include "compare.hh"

#include <stdio.h>

namespace adt
{

enum class AVL_ORDER { PRE, IN, POST };

template<typename T>
struct AVLNode
{
    T data {};
    short height = 0;
    AVLNode* pParent = nullptr;
    AVLNode* pLeft = nullptr;
    AVLNode* pRight = nullptr;
};

template<typename T>
struct AVL
{
    Allocator* pAlloc = nullptr;
    AVLNode<T>* pRoot = nullptr;

    AVL() = default;
    AVL(Allocator* pA) : pAlloc{pA} {}
};


template<typename T>
AVLNode<T>*
AVLNodeAlloc(Allocator* pA, const T& data)
{
    auto* pNew = (AVLNode<T>*)alloc(pA, 1, sizeof(AVLNode<T>));
    pNew->data = data;
    return pNew;
}

template<typename T>
inline short
AVLNodeHeight(AVLNode<T>* p)
{
    return (p ? p->height : -1);
}

template<typename T>
inline short
AVLBalance(AVLNode<T>* p)
{
    return AVLNodeHeight(p->pLeft) - AVLNodeHeight(p->pRight);
}

template<typename T>
inline void
AVLUpdateHeight(AVLNode<T>* p)
{
    short lh = AVLNodeHeight(p->pLeft);
    short rh = AVLNodeHeight(p->pRight);
    p->height = 1 + max(lh, rh);
}

/*            (P)                    (P)
 *             |                      |
 *            (A)    RightRotate     (B)
 *           /  \    ---------->    /  \
 *         (B)  (C)               (D)  (A)
 *        /  \                        /  \
 *      (D)  (E)                    (E)  (C)     */
template<typename T>
inline AVLNode<T>*
RightRotate(AVL<T>* s, AVLNode<T>* a)
{
    auto p = a->pParent;
    auto b = a->pLeft;

    a->pLeft = b->pRight;

    if (b->pRight) b->pRight->pParent = a;

    b->pRight = a;
    a->pParent = b;
    b->pParent = p;

    if (p)
    {
        if (p->pLeft == a) p->pLeft = b;
        else p->pRight = b;
    } else s->pRoot = b;

    AVLUpdateHeight(b);
    AVLUpdateHeight(a);
    return b;
}

/*         (P)                      (P)
 *          |                        |
 *         (B)     LeftRotate       (A)
 *        /  \     --------->      /  \
 *      (D)  (A)                 (B)  (C)
 *          /  \                /  \
 *        (E)  (C)            (D)  (E)          */
template<typename T>
inline AVLNode<T>*
LeftRotate(AVL<T>* s, AVLNode<T>* b)
{
    auto p = b->pParent;
    auto a = b->pRight;

    b->pRight = a->pLeft;

    if (a->pLeft) a->pLeft->pParent = b;

    a->pLeft = b;
    b->pParent = a;
    a->pParent = p;

    if (p)
    {
        if (p->pLeft == b) p->pLeft = a;
        else p->pRight = a;
    } else s->pRoot = a;

    AVLUpdateHeight(a);
    AVLUpdateHeight(b);
    return a;
}

template<typename T>
inline AVLNode<T>*
AVLLeftLeftCase(AVL<T>* s, AVLNode<T>* node)
{
    return RightRotate(s, node);
}

template<typename T>
inline AVLNode<T>*
AVLLeftRightCase(AVL<T>* s, AVLNode<T>* p)
{
    p->pLeft = LeftRotate(s, p->pLeft);
    return RightRotate(s, p);
}

template<typename T>
inline AVLNode<T>*
AVLRightRightCase(AVL<T>* s, AVLNode<T>* node)
{
    return LeftRotate(s, node);
}

template<typename T>
inline AVLNode<T>*
AVLRightLeftCase(AVL<T>* s, AVLNode<T>* p)
{
    p->pRight = RightRotate(s, p->pRight);
    return LeftRotate(s, p);
}

template <class T>
inline AVLNode<T>*
AVLMin(AVLNode<T>* p)
{
    assert(p && "Min of nullptr");
    while (p->pLeft) p = p->pLeft;
    return p;
}

template <class T>
inline AVLNode<T>*
AVLMax(AVLNode<T>* p)
{
    assert(p && "Max of nullptr");
    while (p->pRight) p = p->pRight;
    return p;
}

template<typename T>
inline void
AVLTransplant(AVL<T>* s, AVLNode<T>* u, AVLNode<T>* v)
{
    if (!u->pParent) s->pRoot = v;
    else if (u == u->pParent->pLeft) u->pParent->pLeft = v;
    else u->pParent->pRight = v;

    if (v) v->pParent = u->pParent;
}

template<typename T>
inline void
AVLRebalance(AVL<T>* s, AVLNode<T>* p)
{
    while (p)
    {
        short diff = AVLBalance(p);

        if (diff <= -2)
        {
            if (AVLNodeHeight(p->pRight->pRight) < AVLNodeHeight(p->pRight->pLeft))
                p = AVLRightLeftCase(s, p);
            else p = AVLRightRightCase(s, p);

        }
        else if (diff >= 2)
        {
            if (AVLNodeHeight(p->pLeft->pLeft) < AVLNodeHeight(p->pLeft->pRight))
                p = AVLLeftRightCase(s, p);
            else p = AVLLeftLeftCase(s, p);
        }

        AVLUpdateHeight(p);
        p = p->pParent;
    }
}

template<typename T>
inline void
AVLRemove(AVL<T>* s, AVLNode<T>* d)
{
    AVLNode<T>* succ, * toBalance;

    assert(d && "removing nullptr");

    /* root is the single element case */
    if (!d->pParent && !d->pRight && !d->pLeft)
    {
        free(s->pAlloc, d);
        s->pRoot = nullptr;
        return;
    }

    if (!d->pLeft)
    {
        toBalance = d->pParent;
        AVLTransplant(s, d, d->pRight);

        if (!toBalance) toBalance = d->pRight;

        free(s->pAlloc, d);
    }
    else if (!d->pRight)
    {
        toBalance = d->pParent;
        AVLTransplant(s, d, d->pLeft);

        if (!toBalance) toBalance = d->pLeft;

        free(s->pAlloc, d);
    }
    else
    {
        succ = AVLMin(d->pRight);

        if (succ->pParent != d)
        {
            toBalance = succ->pParent;
            AVLTransplant(s, succ, succ->pRight);
            succ->pRight = d->pRight;
            succ->pRight->pParent = succ;
        }
        else
        {
            if (succ->pRight) toBalance = succ->pRight;
            else toBalance = succ;
        }

        AVLTransplant(s, d, succ);

        succ->pLeft = d->pLeft;
        succ->pLeft->pParent = succ;

        free(s->pAlloc, d);
    }

    AVLUpdateHeight(toBalance);
    AVLRebalance(s, toBalance);
}

template<typename T>
inline AVLNode<T>*
AVLInsert(AVL<T>* s, AVLNode<T>* pNew)
{
    AVLNode<T>** ppSelf = &s->pRoot, * pParent = nullptr;
    s64 comp = 0;
    while (true)
    {
        AVLNode<T>* n = *ppSelf;
        if (!n)
        {
            pNew->height = 0;
            pNew->pParent = pParent;
            pNew->pLeft = pNew->pRight = nullptr;
            *ppSelf = pNew;
            break;
        }

        comp = compare(pNew->data, n->data);

        if (comp == 0) return n;

        if (comp < 0)
        {
            pParent = *ppSelf;
            ppSelf = &n->pLeft;
        }
        else
        {
            pParent = *ppSelf;
            ppSelf = &n->pRight;
        }
    }

    AVLRebalance(s, pNew);
    return pNew;
}

template<typename T>
inline AVLNode<T>*
AVLInsert(AVL<T>* s, const T& data)
{
    auto* pNew = AVLNodeAlloc(s->pAlloc, data);
    return AVLInsert(s, pNew);
}

/* early return if pfn returns true */
template<typename T>
inline void
AVLTraverse(
    AVLNode<T>* p,
    bool (*pfn)(AVLNode<T>*, void*),
    void* pUserData,
    enum AVL_ORDER order
)
{
    if (p)
    {
        switch (order)
        {
            case AVL_ORDER::PRE:
                if (pfn(p, pUserData)) return;
                AVLTraverse(p->pLeft, pfn, pUserData, order);
                AVLTraverse(p->pRight, pfn, pUserData, order);
                break;
            case AVL_ORDER::IN:
                AVLTraverse(p->pLeft, pfn, pUserData, order);
                if (pfn(p, pUserData)) return;
                AVLTraverse(p->pRight, pfn, pUserData, order);
                break;
            case AVL_ORDER::POST:
                AVLTraverse(p->pLeft, pfn, pUserData, order);
                AVLTraverse(p->pRight, pfn, pUserData, order);
                if (pfn(p, pUserData)) return;
                break;
        }
    }
}

template<typename T>
inline AVLNode<T>*
AVLSearch(AVLNode<T>* p, const T& data)
{
    if (p)
    {
        if (data == p->data) return p;
        else if (data < p->data) return AVLSearch(p->pLeft, data);
        else return AVLSearch(p->pRight, data);
    } else return nullptr;
}

template<typename T>
inline short
AVLDepth(AVLNode<T>* p)
{
    if (p)
    {
        short l = AVLDepth(p->pLeft);
        short r = AVLDepth(p->pRight);
        return 1 + max(l, r);
    } else return 0;
}

template<typename T>
inline void
AVLPrintNodes(
    Allocator* pA,
    const AVL<T>* s,
    const AVLNode<T>* pNode,
    void (*pfnPrint)(const AVLNode<T>*, void*),
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

        AVLPrintNodes(pA, s, pNode->pLeft, pfnPrint, pFnData, pF, sCat, true);
        AVLPrintNodes(pA, s, pNode->pRight, pfnPrint, pFnData, pF, sCat, false);
    }
}

} /* namespace adt */
