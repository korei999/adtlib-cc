#pragma once

#include "IAllocator.hh"
#include "defer.hh"
#include "print.inc"

namespace adt
{

template<typename T>
struct SplayTree;

template<typename T>
struct SplayTreeNode
{
    using Node = SplayTreeNode<T>;

    /* */

    Node* m_pLeft {};
    Node* m_pRight {};
    ADT_NO_UNIQUE_ADDRESS T m_data {};

    /* */

    template<typename ...ARGS>
    static Node*
    alloc(IAllocator* pAlloc, ARGS&&... args)
    {
        Node* p = pAlloc->mallocV<Node>(1);
        new(&p->m_data) T (std::forward<ARGS>(args)...);
        p->m_pLeft = p->m_pRight = nullptr;
        return p;
    };

    /* */

    T& data() noexcept { return data; }
    const T& data() const noexcept { return data; }

    void print(IAllocator* pAlloc, FILE* pF, const StringView svPrefix = "");
};

template<typename T>
struct SplayTreeThreeNodes
{
    SplayTreeNode<T>* grand;
    SplayTreeNode<T>* parent;
    SplayTreeNode<T>* node;
};

template<typename T>
struct SplayTree
{
    using Node = SplayTreeNode<T>;
    using Three = SplayTreeThreeNodes<T>;
    static_assert(std::is_same_v<Node, typename SplayTreeNode<T>::Node>);

    /* */

    Node* m_pRoot;

    /* */

    SplayTree() : m_pRoot {nullptr} {}

    /* */

    static inline void print(
        IAllocator* pAlloc,
        FILE* pF,
        const Node* pNode,
        const StringView svPrefix = "",
        bool bLeft = false,
        bool bHasRightSibling = false /* Fix for trailing prefixes. */
    );

    /* */

    void print(
        IAllocator* pAlloc,
        FILE* pF,
        const StringView svPrefix = ""
    ) { print(pAlloc, pF, m_pRoot, svPrefix); }

    Node* root() noexcept { return m_pRoot; }

    template<typename ...ARGS>
    Three insert(IAllocator* pAlloc, ARGS&&... args);

protected:

    Three insertNode(Node* p) noexcept;
    Node* splay(Node* p);
    Node* rotateRight(Node** pp);
    Node* rotateLeft(Node** pp);
};

template<typename T>
inline void
SplayTreeNode<T>::print(IAllocator* pAlloc, FILE* pF, const StringView svPrefix)
{
    SplayTree<T>::print(pAlloc, pF, this, svPrefix);
}

template<typename T>
inline SplayTree<T>::Three
SplayTree<T>::insertNode(Node* p) noexcept
{
    ADT_ASSERT(p != nullptr, "");

    Node** ppWalk = &m_pRoot;
    Node* pGrandParent = nullptr;
    Node* pParent = nullptr;

    while (*ppWalk)
    {
        isize cmp = utils::compare(p->m_data, (*ppWalk)->m_data);
        pGrandParent = pParent;
        pParent = *ppWalk;

        if (cmp == 0) return Three {.grand = pGrandParent, .parent = pParent, .node = *ppWalk};
        else if (cmp < 0) ppWalk = &(*ppWalk)->m_pLeft;
        else ppWalk = &(*ppWalk)->m_pRight;
    }

    *ppWalk = p;

    return Three {
        .grand = pGrandParent,
        .parent = pParent,
        .node = p,
    };
}

template<typename T>
template<typename ...ARGS>
SplayTree<T>::Three
SplayTree<T>::insert(IAllocator* pAlloc, ARGS&&... args)
{
    Three pInserted = insertNode(Node::alloc(pAlloc, std::forward<ARGS>(args)...));
    /*m_pRoot = splay(pInserted);*/

    return pInserted;
}

template<typename T>
inline SplayTree<T>::Node*
SplayTree<T>::splay(Node* x)
{
    while (x->m_pParent)
    {
        if (!x->m_pParent->m_pParent) /* Zig (near root) */
        {
            if (x->m_pParent->m_pLeft == x)
                x = rotateRight(&x->m_pParent);
            else x = rotateLeft(&x->m_pParent);
        }
        else if (x->m_pParent->m_pRight == x) /* zig zag left y right z */
        {
        }
    }

    return x;
}

template<typename T>
inline SplayTree<T>::Node*
SplayTree<T>::rotateRight(Node** pp)
{
    Node* Y = (*pp);
    Node* X = Y->m_pLeft;
    Node* b = X->m_pRight;

    Y->m_pLeft = b;
    X->m_pRight = Y;

    return *pp = X;
}

template<typename T>
inline SplayTree<T>::Node*
SplayTree<T>::rotateLeft(Node** pp)
{
    Node* X = (*pp);
    Node* Y = X->m_pRight;
    Node* b = Y->m_pLeft;

    X->m_pRight = b;
    if (b) b->m_pParent = X;
    Y->m_pLeft = X;

    return *pp = Y;
}

template<typename T>
inline void
SplayTree<T>::print(
    IAllocator* pAlloc,
    FILE* pF,
    const Node* pNode,
    const StringView svPrefix,
    bool bLeft,
    bool bHasRightSibling
)
{
    if (pNode)
    {
        print::toFILE(pAlloc, pF, "{}{} {}\n", svPrefix, bLeft ? "├──" : "└──", pNode->m_data);

        String sCat = StringCat(pAlloc, svPrefix, bHasRightSibling && bLeft ? "│   " : "    ");
        ADT_DEFER( pAlloc->free(sCat.m_pData) );

        print(pAlloc, pF, pNode->m_pLeft, sCat, true, pNode->m_pRight);
        print(pAlloc, pF, pNode->m_pRight, sCat, false);
    }
}

namespace print
{

template<typename T>
inline isize
formatToContext(Context ctx, FormatArgs fmtArgs, const SplayTreeNode<T>* const x)
{
    if (x) return formatToContext(ctx, fmtArgs, x->m_data);
    else return formatToContext(ctx, fmtArgs, nullptr);
}

template<typename T>
inline isize
formatToContext(Context ctx, FormatArgs fmtArgs, const SplayTreeThreeNodes<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::SQUARE_BRACKETS;
    return formatToContextVariadic(ctx, fmtArgs, x.grand, x.parent, x.node);
}

} /* namespace format */

} /* namespace adt */
