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
    void destroy() noexcept;
    void insert(isize pos, const StringView sv);
    void remove(isize pos, isize size);
    void defragment();

    [[nodiscard]] String toString(IAllocator* pAlloc);
};

inline
PieceList::PieceList(RefCountedPtr<StringM> rcpS)
{
    m_lPieces.insert({.m_rcpS = rcpS.ref(), .m_pos = 0, .m_size = rcpS->m_size});
    m_size = rcpS->m_size;
}

inline void
PieceList::insert(isize pos, const StringView sv)
{
    Node** ppNode = &m_lPieces.m_pHead;

    while (*ppNode && pos >= (*ppNode)->data.m_size)
    {
        pos -= (*ppNode)->data.m_size;
        ppNode = &(*ppNode)->pNext;
    }

    auto* pNode = *ppNode;

    auto clNewNode = [&] -> Node* {
        return Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sv),
            .m_pos = 0,
            .m_size = sv.m_size,
        });
    };

    if (!pNode) /* append case */
    {
        *ppNode = clNewNode();
        m_size += sv.m_size;
        return;
    }

    auto& rPiece = pNode->data;
    ADT_ASSERT(pos >= 0 && pos < rPiece.m_size, "pos: {}, rPiece.size: {}", pos, rPiece.m_size);

    if (pos > 0) /* split case */
    {
        Node* pNew = clNewNode();
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
    }
    else /* prepend case */
    {
        Node* pNew = clNewNode();
        ADT_ASSERT(pos == 0, "pos: {}", pos);
        pNew->pNext = pNode;
        *ppNode = pNew;
    }

    m_size += sv.m_size;
}


inline void
PieceList::remove(isize pos, isize size)
{
    ADT_ASSERT(!m_lPieces.empty(), "");
    ADT_ASSERT(pos >= 0 && pos < m_size && size > 0 && (pos + size) <= m_size, "m_size: {}, pos: {}, size: {}", m_size, pos, size);

    Node** ppNode = &m_lPieces.m_pHead;

    while (*ppNode && pos >= (*ppNode)->data.m_size)
    {
        pos -= (*ppNode)->data.m_size;
        ppNode = &(*ppNode)->pNext;
    }

    const isize fullSize = size;

    if (pos == 0)
    {
        Piece& rPiece = (*ppNode)->data;

        const isize nShed = utils::min(rPiece.m_size, size);
        rPiece.m_size -= nShed;

        size -= nShed;

        if (rPiece.m_size <= 0)
        {
            Node* pNode = *ppNode;
            *ppNode = pNode->pNext;
            rPiece.m_rcpS.unref();
            StdAllocator::inst()->free(pNode);
        }
    }
    else
    {
        Piece& rPiece = (*ppNode)->data;

        if (size < rPiece.m_size - pos) /* split case */
        {
            Node* pLeftNode = Node::alloc(StdAllocator::inst(), Piece{
                .m_rcpS = rPiece.m_rcpS.ref(),
                .m_pos = rPiece.m_pos,
                .m_size = pos,
            });
            Node* pNode = *ppNode;
            *ppNode = pLeftNode;
            pLeftNode->pNext = pNode;

            pNode->data.m_pos += (pos + size);
            pNode->data.m_size -= (pos + size);

            m_size -= fullSize;
            return;
        }
        else if (size >= rPiece.m_size - pos)
        {
            size -= rPiece.m_size - pos;
            rPiece.m_size = pos;
            ppNode = &(*ppNode)->pNext;
        }
    }

    while (size > 0)
    {
        Piece& rPiece = (*ppNode)->data;

        const isize nShed = utils::min(rPiece.m_size, size);
        rPiece.m_size -= nShed;
        rPiece.m_pos += nShed;

        size -= nShed;

        if (rPiece.m_size <= 0)
        {
            Node* pNode = *ppNode;
            *ppNode = pNode->pNext;

            rPiece.m_rcpS.unref();
            StdAllocator::inst()->free(pNode);
        }
    }

    m_size -= fullSize;
}

inline void
PieceList::destroy() noexcept
{
    m_lPieces.destroy([](Piece* p) { p->m_rcpS.unref(); });
    m_size = 0;
}

inline void
PieceList::defragment()
{
    if (m_size <= 0) return;

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

inline String
PieceList::toString(IAllocator* pAlloc)
{
    if (m_size <= 0) return {};

    StringM sRet;
    sRet.m_pData = pAlloc->zallocV<char>(m_size + 1);
    sRet.m_size = m_size;

    isize i = 0;
    for (auto e : m_lPieces)
    {
        ::memcpy(sRet.m_pData + i, e.view().m_pData, e.m_size);
        i+= e.m_size;
    }

    return sRet;
}

} /* namespace adt */
