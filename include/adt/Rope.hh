#pragma once

#include "String.hh"
#include "RefCount.hh"

namespace adt
{

struct RopeNode
{
    RopeNode* pLeft {};
    RopeNode* pRight {};
    RopeNode* pParent {};
    isize weight {};
};

struct RopeLeaf
{
    RopeNode* pLeft {};
    RopeNode* pRight {};
    RopeNode* pParent {};
    RefCountedPtr<StringM> rcpS {};
};

struct Rope
{
    using Node = RopeNode;
    using Leaf = RopeLeaf;

    /* */

    RopeNode* m_pRoot {};

    /* */

    Leaf* insert(const StringView sv, isize atI);

    Leaf* insert(Leaf* pNode, isize atI) noexcept;

    void destroy() noexcept;

    template<typename CL> static void traversePost(Node* p, CL clVisit);
};

inline RopeLeaf*
Rope::insert(const StringView sv, isize atI)
{
    auto rcp = RefCountedPtr<StringM>::allocWithDeleter(+[](StringM* p) { p->destroy(); }, sv);
    return insert(StdAllocator::inst()->alloc<Leaf>( Leaf{.rcpS = rcp}), atI);
}

inline RopeLeaf*
Rope::insert(Leaf* pNode, isize atI) noexcept
{
    isize weightedKey = atI;

    if (!m_pRoot)
    {
        m_pRoot = reinterpret_cast<Node*>(pNode);
        return pNode;
    }

    return nullptr;
}

inline void
Rope::destroy() noexcept
{
    traversePost(m_pRoot, [&](Node* p) {
        if (!p->pLeft || !p->pRight)
            reinterpret_cast<Leaf*>(p)->rcpS.unref();
        StdAllocator::inst()->free(p);
    });
}

template<typename CL>
void inline
Rope::traversePost(Node* p, CL clVisit)
{
    if (p)
    {
        traversePost(p->pLeft, clVisit);
        traversePost(p->pRight, clVisit);
        clVisit(p);
    }
}

} /* namespace adt */
