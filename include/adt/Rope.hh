#pragma once

#include "String.hh"
#include "RefCount.hh"

#include "logs.hh"

namespace adt
{

struct RopeNode
{
    RopeNode* m_pLeft {};
    RopeNode* m_pRight {};
    RopeNode* m_pParent {};
    isize m_weight {};

    /* */

    isize weight() const noexcept { return m_weight; }
};

struct RopeLeaf
{
    RopeNode* m_pLeft {};
    RopeNode* m_pRight {};
    RopeNode* m_pParent {};
    RefCountedPtr<StringM> m_rcpS {};
    isize m_firstI {};
    isize m_lastI {};

    /* */

    isize weight() const noexcept { return (m_lastI - m_firstI) + 1; } /* Aka the size of this piece. */
    StringView piece() const noexcept { return m_rcpS->subString(m_firstI, weight()); }
};

struct Rope
{
    using Node = RopeNode;
    using Leaf = RopeLeaf;

    /* */

    Node* m_pRoot {};
    isize m_totalSize {};

    /* */

    Node* root() noexcept { return m_pRoot; }
    const Node* root() const noexcept { return m_pRoot; }

    char charAt(isize i) const noexcept;

    Leaf* insert(const StringView sv, isize atI);

    Leaf* insert(Leaf* pNode, isize atI);

    Pair<Leaf*, isize> iToLeaf(isize atI);

    void destroy() noexcept;

    void printTree(IAllocator* pAlloc, FILE* pF) { printTree(pAlloc, pF, m_pRoot); }

    /* */

    template<typename CL> static void traversePre(Node* p, CL clVisit);
    template<typename CL> static void traverseIn(Node* p, CL clVisit);
    template<typename CL> static void traversePost(Node* p, CL clVisit);

    static void printTree(
        IAllocator* pAlloc,
        FILE* pF,
        const Node* pNode,
        const StringView svPrefix = "",
        bool bLeft = false,
        bool bHasRightSibling = false /* Fix for trailing prefixes. */
    );

    static isize realWeight(Node* pNode) noexcept;

    /* */

protected:
    static void fixWeights(Node* pNode, isize addedWeight) noexcept;
    static void insertSplit(Leaf** ppLeaf, Leaf* pNew, isize weightedKey);
    static void insertAppend(Leaf** ppLeaf, Leaf* pNew, isize weightedKey);
    static void insertPrepend(Leaf** ppLeaf, Leaf* pNew, isize weightedKey);
};

inline char
Rope::charAt(isize i) const noexcept
{
    if (!m_pRoot) return '\0';

    Node* walk = m_pRoot;

    isize weightedKey = i;
    while (walk->m_pLeft)
    {
        ADT_ASSERT(walk->m_pRight, "");

        if (weightedKey < walk->m_weight)
        {
            walk = walk->m_pLeft;
        }
        else
        {
            weightedKey -= walk->m_weight;
            walk = walk->m_pRight;
        }
    }

    return reinterpret_cast<Leaf*>(walk)->piece()[weightedKey];
}

inline RopeLeaf*
Rope::insert(const StringView sv, isize atI)
{
    if (sv.empty()) return nullptr;
    defer( m_totalSize += sv.size() );

    auto rcp = RefCountedPtr<StringM>::allocWithDeleter(+[](StringM* p) { p->destroy(); }, sv);
    return insert(
        StdAllocator::inst()->alloc<Leaf>(
            Leaf{.m_rcpS = rcp, .m_firstI = 0, .m_lastI = rcp->size() - 1}
        ),
        atI
    );
}

inline RopeLeaf*
Rope::insert(Leaf* pNew, isize atI)
{
    if (!m_pRoot)
    {
        LOG_BAD("---ROOT CASE---: atI: {}\n", atI);
        ADT_ASSERT(atI == 0, "{}", atI);
        m_pRoot = reinterpret_cast<Node*>(pNew);
        return pNew;
    }

    Node** walk = &m_pRoot;

    isize weightedKey = atI;
    while ((*walk)->m_pLeft)
    {
        if (weightedKey < (*walk)->m_weight)
        {
            walk = &(*walk)->m_pLeft;
        }
        else
        {
            weightedKey -= (*walk)->m_weight;
            walk = &(*walk)->m_pRight;
        }
    }

    ADT_ASSERT(*walk != nullptr && !(*walk)->m_pRight && !(*walk)->m_pLeft, "");

    const isize leafWeight = reinterpret_cast<Leaf*>(*walk)->weight();

    if (weightedKey > 0 && weightedKey < leafWeight)
    {
        insertSplit(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }
    else if (weightedKey == leafWeight)
    {
        insertAppend(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }
    else
    {
        ADT_ASSERT(weightedKey == 0, "{}", weightedKey);
        insertPrepend(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }

    return nullptr;
}

inline Pair<RopeLeaf*, isize>
Rope::iToLeaf(isize atI)
{
    return {};
}

inline void
Rope::destroy() noexcept
{
    traversePost(m_pRoot, [](Node* p) {
        if (!p->m_pLeft)
        {
            ADT_ASSERT(!p->m_pRight, "");
            reinterpret_cast<Leaf*>(p)->m_rcpS.unref();
        }
        StdAllocator::inst()->free(p);
    });
}

inline void
Rope::fixWeights(Node* pNode, isize addedWeight) noexcept
{
    ADT_ASSERT(pNode != nullptr, "");

    Node* pPrev = nullptr;
    while (pNode->m_pParent)
    {
        pPrev = pNode;
        pNode = pNode->m_pParent;

        if (pNode->m_pLeft == pPrev)
            pNode->m_weight += addedWeight;
    }
}

/* Split case:
 *   /\                          (pTopWeight)
 *  /  \   ------->              /         \
 * /____\                       /           /\
 * (ppLeaf)                    /           /__\(pRightLeaf(ppLeaf(rightPiece)))
 *                       (pLeftWeight)
 *                       /          \
 *                     /\            /\
 * (ppLeaf(leftPiece))/__\          /__\(pNew)                */
inline void
Rope::insertSplit(Leaf** ppLeaf, Leaf* pNew, isize weightedKey)
{
    Leaf* pLeaf = *ppLeaf;
    Node* pPar = pLeaf->m_pParent;

    auto* pTopWeight = StdAllocator::inst()->mallocV<Node>(1);
    auto* pLeftWeight = StdAllocator::inst()->mallocV<Node>(1);
    auto* pRightLeaf = StdAllocator::inst()->mallocV<Leaf>(1);

    pRightLeaf->m_rcpS = pLeaf->m_rcpS.ref();
    pRightLeaf->m_lastI = pLeaf->m_lastI;
    pRightLeaf->m_firstI = pLeaf->m_firstI + weightedKey;

    pLeaf->m_lastI = pLeaf->m_firstI + (weightedKey - 1);

    { /* pTopWeight */
        pTopWeight->m_pParent = pPar;
        if (pPar)
        {
            if (pPar->m_pLeft == reinterpret_cast<Node*>(pLeaf)) pPar->m_pLeft = pTopWeight;
            else pPar->m_pRight = pTopWeight;
        }
        pTopWeight->m_pRight = reinterpret_cast<Node*>(pRightLeaf);
        pTopWeight->m_pLeft = pLeftWeight;
        pTopWeight->m_weight = pLeaf->weight() + pNew->m_rcpS->size();
    }

    { /* pLeftWeight */
        pLeftWeight->m_pParent = pTopWeight;
        pLeftWeight->m_pLeft = reinterpret_cast<Node*>(pLeaf);
        pLeftWeight->m_pRight = reinterpret_cast<Node*>(pNew);
        pLeftWeight->m_weight = pLeaf->weight();
    }

    /* pRightLeaf */
    pRightLeaf->m_pParent = pTopWeight;
    pRightLeaf->m_pLeft = pRightLeaf->m_pRight = nullptr;

    /* pLeaf */
    pLeaf->m_pParent = pLeftWeight;

    /* pNew */
    pNew->m_pParent = pLeftWeight;

    fixWeights(pTopWeight, pNew->weight());

    *ppLeaf = reinterpret_cast<Leaf*>(pTopWeight);
}

/* Append case:
 *   /\
 *  /  \   ---->        (pWeight)
 * /____\               /       \
 * (ppLeaf)           /\         /\
 *                   /__\       /__\(pNew)
 *                (ppLeaf)                        */
inline void
Rope::insertAppend(Leaf** ppLeaf, Leaf* pNew, isize weightedKey)
{
    Leaf* pLeaf = *ppLeaf;
    Node* pPar = pLeaf->m_pParent;

    auto* pWeight = StdAllocator::inst()->mallocV<Node>(1);

    { /* pWeight */
        pWeight->m_pParent = pPar;
        if (pPar)
        {
            if (pPar->m_pLeft == reinterpret_cast<Node*>(pLeaf))
                pPar->m_pLeft = pWeight;
            else pPar->m_pRight = pWeight;
        }
        pWeight->m_pLeft = reinterpret_cast<Node*>(pLeaf);
        pWeight->m_pRight = reinterpret_cast<Node*>(pNew);
        pWeight->m_weight = pLeaf->weight();
    }

    /* pLeaf */
    pLeaf->m_pParent = pWeight;

    /* pNew */
    pNew->m_pParent = pWeight;

    fixWeights(pWeight, pNew->weight());

    *ppLeaf = reinterpret_cast<Leaf*>(pWeight);
}

/* Prepend case:
 *   /\
 *  /  \   ---->        (pWeight)
 * /____\               /       \
 * (ppLeaf)           /\         /\
 *                   /__\       /__\(ppLeaf)
 *                (pNew)                        */
inline void
Rope::insertPrepend(Leaf** ppLeaf, Leaf* pNew, isize weightedKey)
{
    Leaf* pLeaf = *ppLeaf;
    Node* pPar = pLeaf->m_pParent;

    Node* pWeight = StdAllocator::inst()->mallocV<Node>(1);

    { /* pWeight */
        pWeight->m_pParent = pPar;
        if (pPar)
        {
            if (pPar->m_pLeft == reinterpret_cast<Node*>(pLeaf))
                pPar->m_pLeft = pWeight;
            else pPar->m_pRight = pWeight;
        }
        pWeight->m_pLeft = reinterpret_cast<Node*>(pNew);
        pWeight->m_pRight = reinterpret_cast<Node*>(pLeaf);
        pWeight->m_weight = pNew->weight();
    }

    /* pLeaf */
    pLeaf->m_pParent = pWeight;

    /* pNew */
    pNew->m_pParent = pWeight;

    fixWeights(pWeight, pNew->weight());

    *ppLeaf = reinterpret_cast<Leaf*>(pWeight);
}

template<typename CL>
void inline
Rope::traversePre(Node* p, CL clVisit)
{
    if (p)
    {
        clVisit(p);
        traversePre(p->m_pLeft, clVisit);
        traversePre(p->m_pRight, clVisit);
    }
}

template<typename CL>
void inline
Rope::traverseIn(Node* p, CL clVisit)
{
    if (p)
    {
        traverseIn(p->m_pLeft, clVisit);
        clVisit(p);
        traverseIn(p->m_pRight, clVisit);
    }
}

template<typename CL>
void inline
Rope::traversePost(Node* p, CL clVisit)
{
    if (p)
    {
        traversePost(p->m_pLeft, clVisit);
        traversePost(p->m_pRight, clVisit);
        clVisit(p);
    }
}

inline void
Rope::printTree(
    IAllocator* pAlloc,
    FILE* pF,
    const RopeNode* pNode,
    const StringView svPrefix,
    bool bLeft,
    bool bHasRightSibling
)
{
    if (pNode)
    {
        if (!pNode->m_pLeft)
        {
            const Leaf* pLeaf = reinterpret_cast<const Leaf*>(pNode);
            print::toFILE(pAlloc, pF, "{}{} '{}'\n", svPrefix, bLeft ? "├──" : "└──", pLeaf->piece());
        }
        else
        {
            print::toFILE(pAlloc, pF, "{}{} {}\n", svPrefix, bLeft ? "├──" : "└──", pNode->m_weight);
        }

        String sCat = StringCat(pAlloc, svPrefix, bHasRightSibling && bLeft ? "│   " : "    ");
        ADT_DEFER( pAlloc->free(sCat.m_pData) );

        printTree(pAlloc, pF, pNode->m_pLeft, sCat, true, pNode->m_pRight);
        printTree(pAlloc, pF, pNode->m_pRight, sCat, false, false);
    }
}

inline isize
Rope::realWeight(Node* pNode) noexcept
{
    if (!pNode->m_pLeft)
        return reinterpret_cast<Leaf*>(pNode)->weight();
    else return pNode->m_weight;
}

} /* namespace adt */
