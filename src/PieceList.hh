#include "adt/String-inl.hh"
#include "adt/RefCount.hh"
#include "adt/SList.hh"

namespace adt
{

struct PieceList
{
    struct Piece
    {
        RefCountedPtr<StringM> m_rcpS {};
        isize m_pos {};
        isize m_size {};

        /* */

        StringView view() noexcept { return {m_rcpS->data() + m_pos, m_size}; }
    };

    using List = SList<Piece>;
    using Node = List::Node;

    /* */

    SListM<Piece> m_lPieces {};
    isize m_size {};

    /* */

    PieceList() noexcept = default;

    PieceList(RefCountedPtr<StringM> rcpS);

    /* */

    isize size() const noexcept { return m_size; }
    void insert(isize pos, const StringView sv);
    void destroy() noexcept;
    void defragment();
};

inline
PieceList::PieceList(RefCountedPtr<StringM> rcpS)
{
    m_lPieces.insert({.m_rcpS = rcpS.ref(), .m_pos = 0, .m_size = rcpS->m_size});
    m_size += rcpS->m_size;
}

inline void
PieceList::insert(isize pos, const StringView sv)
{
    auto** ppNode = &m_lPieces.m_pHead;
    SList<Piece>::Node* pPrev = nullptr;

again:
    if (auto* p = *ppNode; *ppNode && pos >= p->data.m_size)
    {
        pos -= p->data.m_size;
        pPrev = p;
        ppNode = &p->pNext;
        goto again;
    }

    auto* pNode = *ppNode;

    Node* pNew = Node::alloc(StdAllocator::inst(), Piece{
        .m_rcpS = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sv),
        .m_pos = 0,
        .m_size = sv.m_size,
    });
    m_size += sv.m_size;

    if (!pNode) /* append case */
    {
        *ppNode = pNew;
        return;
    }

    auto& rPiece = pNode->data;
    ADT_ASSERT(pos >= 0 && pos < rPiece.m_size, "pos: {}, rPiece.size: {}", pos, rPiece.m_size);

    if (pos > 0) /* split case */
    {
        pNew->pNext = pNode;

        Node* pLeftNode = Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = rPiece.m_rcpS.ref(),
            .m_pos = rPiece.m_pos,
            .m_size = pos,
        });
        pLeftNode->pNext = pNew;
        *ppNode = pLeftNode;

        rPiece.m_size -= pos;
        rPiece.m_pos += pos;

        if (pPrev) pPrev->pNext = pLeftNode;
    }
    else /* prepend case */
    {
        ADT_ASSERT(pos == 0, "pos: {}", pos);
        pNew->pNext = pNode;
        if (pPrev) pPrev->pNext = pNew;
        *ppNode = pNew;
    }
}

inline void
PieceList::destroy() noexcept
{
    m_lPieces.destroy([](Piece* p) { p->m_rcpS.unref(); });
}

inline void
PieceList::defragment()
{
    StringM s;
    s.m_pData = StdAllocator::inst()->zallocV<char>(m_size + 1);
    s.m_size = m_size;

    Piece piece {
        .m_rcpS = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, s),
        .m_pos = 0,
        .m_size = m_size,
    };

    isize i = 0;
    m_lPieces.destroy([&](Piece* p) {
        ::memcpy(s.m_pData + i, p->view().m_pData, p->m_size);
        i += p->m_size;
        p->m_rcpS.unref();
    });

    m_lPieces.insert(piece);
}

} /* namespace adt */
