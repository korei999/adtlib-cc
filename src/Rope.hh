#pragma once

#include "adt/String.hh"
#include "adt/RefCount.hh"

namespace adt
{

struct RopeNode
{
    RopeNode* m_parent {};
    RopeNode* m_left {};
    RopeNode* m_right {};
    isize m_weight {};

    /* */

    bool isLeaf() const noexcept { ADT_ASSERT(!m_left ? !m_right : m_left && m_right, ""); return !m_left; }
    bool notLeaf() const noexcept { ADT_ASSERT(m_left ? bool(m_right) : !m_left && !m_right, ""); return m_left; }
};

struct RopeLeaf : RopeNode
{
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

    isize size() const noexcept { return m_totalSize; }

    Node* root() noexcept { return m_pRoot; }
    const Node* root() const noexcept { return m_pRoot; }

    char charAt(isize i) const noexcept;

    Leaf* insert(const StringView sv, isize atI);

    Leaf* remove(isize atI, isize size) noexcept;

    Leaf* insertNode(Leaf* pNode, isize atI);

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

    while (walk->notLeaf())
    {
        ADT_ASSERT(walk->m_right, "");

        if (i < walk->m_weight)
        {
            walk = walk->m_left;
        }
        else
        {
            i -= walk->m_weight;
            walk = walk->m_right;
        }
    }

    return reinterpret_cast<Leaf*>(walk)->piece()[i];
}

inline RopeLeaf*
Rope::insert(const StringView sv, isize atI)
{
    if (sv.empty()) return nullptr;

    auto rcp = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sv);
    auto* pLeaf = StdAllocator::inst()->alloc<Leaf>(Node{}, rcp, 0, rcp->size() - 1);
    auto* p = insertNode(pLeaf, atI);

    m_totalSize += sv.size();
    return p;
}

inline RopeLeaf*
Rope::remove(isize atI, isize size) noexcept
{
    return nullptr;
}

inline RopeLeaf*
Rope::insertNode(Leaf* pNew, isize atI)
{
    if (!m_pRoot)
    {
        ADT_ASSERT(atI == 0, "{}", atI);
        m_pRoot = static_cast<Node*>(pNew);
        return pNew;
    }

    Node** walk = &m_pRoot;

    isize weightedKey = atI;
    while ((*walk)->notLeaf())
    {
        if (weightedKey < (*walk)->m_weight)
        {
            walk = &(*walk)->m_left;
        }
        else
        {
            weightedKey -= (*walk)->m_weight;
            walk = &(*walk)->m_right;
        }
    }

    const isize leafWeight = static_cast<Leaf*>(*walk)->weight();

    if (weightedKey == leafWeight)
    {
        insertAppend(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }
    else if (weightedKey == 0)
    {
        insertPrepend(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }
    else
    {
        ADT_ASSERT(weightedKey > 0 && weightedKey < leafWeight, "weightedKey: {}, leafWeight: {}", weightedKey, leafWeight);
        insertSplit(reinterpret_cast<Leaf**>(walk), pNew, weightedKey);
        return pNew;
    }

    return nullptr;
}

inline void
Rope::destroy() noexcept
{
    traversePost(m_pRoot, [](Node* p) {
        if (!p->m_left)
        {
            ADT_ASSERT(!p->m_right, "");
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
    while (pNode->m_parent)
    {
        pPrev = pNode;
        pNode = pNode->m_parent;

        if (pNode->m_left == pPrev)
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
    Node* pPar = pLeaf->m_parent;

    auto* pTopWeight = StdAllocator::inst()->mallocV<Node>(1);
    auto* pLeftWeight = StdAllocator::inst()->mallocV<Node>(1);
    auto* pRightLeaf = StdAllocator::inst()->mallocV<Leaf>(1);

    pRightLeaf->m_rcpS = pLeaf->m_rcpS.ref();
    pRightLeaf->m_lastI = pLeaf->m_lastI;
    pRightLeaf->m_firstI = pLeaf->m_firstI + weightedKey;

    pLeaf->m_lastI = pLeaf->m_firstI + (weightedKey - 1);

    { /* pTopWeight */
        pTopWeight->m_parent = pPar;
        if (pPar)
        {
            if (pPar->m_left == static_cast<Node*>(pLeaf)) pPar->m_left = pTopWeight;
            else pPar->m_right = pTopWeight;
        }
        pTopWeight->m_right = static_cast<Node*>(pRightLeaf);
        pTopWeight->m_left = pLeftWeight;
        pTopWeight->m_weight = pLeaf->weight() + pNew->weight();
    }

    { /* pLeftWeight */
        pLeftWeight->m_parent = pTopWeight;
        pLeftWeight->m_left = static_cast<Node*>(pLeaf);
        pLeftWeight->m_right = static_cast<Node*>(pNew);
        pLeftWeight->m_weight = pLeaf->weight();
    }

    /* pRightLeaf */
    pRightLeaf->m_parent = pTopWeight;
    pRightLeaf->m_left = pRightLeaf->m_right = nullptr;

    /* pLeaf */
    pLeaf->m_parent = pLeftWeight;

    /* pNew */
    pNew->m_parent = pLeftWeight;

    fixWeights(pTopWeight, pNew->weight());

    *ppLeaf = static_cast<Leaf*>(pTopWeight);
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
    Node* pPar = pLeaf->m_parent;

    auto* pWeight = StdAllocator::inst()->mallocV<Node>(1);

    { /* pWeight */
        pWeight->m_parent = pPar;
        if (pPar)
        {
            if (pPar->m_left == static_cast<Node*>(pLeaf))
                pPar->m_left = pWeight;
            else pPar->m_right = pWeight;
        }
        pWeight->m_left = static_cast<Node*>(pLeaf);
        pWeight->m_right = static_cast<Node*>(pNew);
        pWeight->m_weight = pLeaf->weight();
    }

    /* pLeaf */
    pLeaf->m_parent = pWeight;

    /* pNew */
    pNew->m_parent = pWeight;

    fixWeights(pWeight, pNew->weight());

    *ppLeaf = static_cast<Leaf*>(pWeight);
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
    Node* pPar = pLeaf->m_parent;

    Node* pWeight = StdAllocator::inst()->mallocV<Node>(1);

    { /* pWeight */
        pWeight->m_parent = pPar;
        if (pPar)
        {
            if (pPar->m_left == static_cast<Node*>(pLeaf))
                pPar->m_left = pWeight;
            else pPar->m_right = pWeight;
        }
        pWeight->m_left = static_cast<Node*>(pNew);
        pWeight->m_right = static_cast<Node*>(pLeaf);
        pWeight->m_weight = pNew->weight();
    }

    /* pLeaf */
    pLeaf->m_parent = pWeight;

    /* pNew */
    pNew->m_parent = pWeight;

    fixWeights(pWeight, pNew->weight());

    *ppLeaf = static_cast<Leaf*>(pWeight);
}

template<typename CL>
void inline
Rope::traversePre(Node* p, CL clVisit)
{
    if (p)
    {
        clVisit(p);
        traversePre(p->m_left, clVisit);
        traversePre(p->m_right, clVisit);
    }
}

template<typename CL>
void inline
Rope::traverseIn(Node* p, CL clVisit)
{
    if (p)
    {
        traverseIn(p->m_left, clVisit);
        clVisit(p);
        traverseIn(p->m_right, clVisit);
    }
}

template<typename CL>
void inline
Rope::traversePost(Node* p, CL clVisit)
{
    if (p)
    {
        traversePost(p->m_left, clVisit);
        traversePost(p->m_right, clVisit);
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
        if (!pNode->m_left)
        {
            const Leaf* pLeaf = static_cast<const Leaf*>(pNode);
            print::toFILE(pAlloc, pF, "{}{} '{}'\n", svPrefix, bLeft ? "├──" : "└──", pLeaf->piece());
        }
        else
        {
            print::toFILE(pAlloc, pF, "{}{} {}\n", svPrefix, bLeft ? "├──" : "└──", pNode->m_weight);
        }

        String sCat = StringCat(pAlloc, svPrefix, bHasRightSibling && bLeft ? "│   " : "    ");
        ADT_DEFER( pAlloc->free(sCat.m_pData) );

        printTree(pAlloc, pF, pNode->m_left, sCat, true, pNode->m_right);
        printTree(pAlloc, pF, pNode->m_right, sCat, false, false);
    }
}

inline isize
Rope::realWeight(Node* pNode) noexcept
{
    if (pNode->isLeaf())
        return reinterpret_cast<Leaf*>(pNode)->weight();
    else return pNode->m_weight;
}

} /* namespace adt */
