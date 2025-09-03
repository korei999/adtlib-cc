#include "adt/RefCount.hh"
#include "adt/List.hh"

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

    using List = List<Piece>;
    using Node = List::Node;

    /* */

    ListM<Piece> m_lPieces {};
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

    [[nodiscard]] Node* posToNode2(isize* pPos) noexcept;
};

inline
PieceList::PieceList(RefCountedPtr<StringM> rcpS)
{
    m_lPieces.pushBack(Piece{.m_rcpS = rcpS.ref(), .m_pos = 0, .m_size = rcpS->m_size});
    m_size = rcpS->m_size;
}

inline void
PieceList::insert(isize pos, const StringView sv)
{
    auto clNewNode = [&] -> Node* {
        return Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sv),
            .m_pos = 0,
            .m_size = sv.m_size,
        });
    };

    if (!m_lPieces.m_pFirst)
    {
        Node* pNew = clNewNode();
        static_cast<List&>(m_lPieces).pushBack(pNew);

        m_size += sv.m_size;
        return;
    }

    Node* pNode = posToNode2(&pos);

    if (pNode->data.m_size == pos) /* append case */
    {
        Node* pNew = clNewNode();
        m_lPieces.insertAfter(pNode, pNew);

        m_size += sv.m_size;
        return;
    }

    auto& rPiece = pNode->data;
    ADT_ASSERT(pos >= 0 && pos < rPiece.m_size, "pos: {}, rPiece.size: {}", pos, rPiece.m_size);

    if (pos > 0) /* split case */
    {
        Node* pLeftNode = Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = rPiece.m_rcpS.ref(),
            .m_pos = rPiece.m_pos,
            .m_size = pos,
        });
        m_lPieces.insertBefore(pNode, pLeftNode);

        Node* pNew = clNewNode();
        m_lPieces.insertAfter(pLeftNode, pNew);

        rPiece.m_size -= pos;
        rPiece.m_pos += pos;
    }
    else /* prepend case */
    {
        ADT_ASSERT(pos == 0, "pos: {}", pos);
        Node* pNew = clNewNode();
        m_lPieces.insertBefore(pNode, pNew);
    }

    m_size += sv.m_size;
}

inline void
PieceList::remove(isize pos, isize size)
{
    ADT_ASSERT(!m_lPieces.empty(), "");
    ADT_ASSERT(pos >= 0 && pos < m_size && size > 0 && (pos + size) <= m_size, "m_size: {}, pos: {}, size: {}", m_size, pos, size);

    Node *pNode = posToNode2(&pos);
    const isize fullSize = size;

    if (pos == 0)
    {
        Piece& rPiece = pNode->data;

        const isize nShed = utils::min(rPiece.m_size, size);
        rPiece.m_size -= nShed;

        size -= nShed;

        if (rPiece.m_size <= 0)
        {
            rPiece.m_rcpS.unref();
            Node* pNext = pNode->pNext;
            m_lPieces.remove(pNode);
            pNode = pNext;
        }
    }
    else
    {
        Piece& rPiece = pNode->data;

        if (size < rPiece.m_size - pos) /* split case */
        {
            Node* pLeftNode = Node::alloc(StdAllocator::inst(), Piece{
                .m_rcpS = rPiece.m_rcpS.ref(),
                .m_pos = rPiece.m_pos,
                .m_size = pos,
            });
            m_lPieces.insertBefore(pNode, pLeftNode);

            pNode->data.m_pos += (pos + size);
            pNode->data.m_size -= (pos + size);

            m_size -= fullSize;
            return;
        }
        else if (size >= rPiece.m_size - pos)
        {
            size -= rPiece.m_size - pos;
            rPiece.m_size = pos;
            pNode = pNode->pNext;
        }
    }

    while (size > 0)
    {
        Piece& rPiece = pNode->data;

        const isize nShed = utils::min(rPiece.m_size, size);
        rPiece.m_size -= nShed;
        rPiece.m_pos += nShed;

        size -= nShed;

        if (rPiece.m_size <= 0)
        {
            Node* pNext = pNode->pNext;
            rPiece.m_rcpS.unref();
            m_lPieces.remove(pNode);
            pNode = pNext;
        }
    }

    m_size -= fullSize;
}

inline void
PieceList::destroy() noexcept
{
    m_lPieces.destroy([&](Piece* p) { p->m_rcpS.unref(); });
    m_size = 0;
}

inline void
PieceList::defragment()
{
    if (m_size <= 0) return;

    StringM sRet;
    sRet.m_pData = StdAllocator::inst()->mallocV<char>(m_size + 1);
    sRet.m_size = m_size;

    Piece piece {
        .m_rcpS = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sRet),
        .m_pos = 0,
        .m_size = m_size,
    };

    isize i = 0;
    m_lPieces.destroy([&](Piece* p) {
        ::memcpy(sRet.m_pData + i, p->view().m_pData, p->m_size);
        i += p->m_size;
        p->m_rcpS.unref();
    });
    sRet.m_pData[i] = '\0';

    static_cast<List&>(m_lPieces).pushBack(Node::alloc(StdAllocator::inst(), piece));
}

inline String
PieceList::toString(IAllocator* pAlloc)
{
    if (m_size <= 0) return {};

    StringM sRet;
    sRet.m_pData = pAlloc->mallocV<char>(m_size + 1);
    sRet.m_size = m_size;

    isize i = 0;
    Node* p = m_lPieces.m_pFirst;
    while (p)
    {
        Node* pNext = p->pNext;

        ::memcpy(sRet.m_pData + i, p->data.view().m_pData, p->data.m_size);
        i += p->data.m_size;

        p = pNext;
    }
    sRet.m_pData[i] = '\0';

    return sRet;
}

inline PieceList::Node*
PieceList::posToNode2(isize* pPos) noexcept
{
    ADT_ASSERT(!m_lPieces.empty(), "");

    Node* pNode = m_lPieces.m_pFirst;
    while (pNode->pNext && *pPos >= pNode->data.m_size)
    {
        *pPos -= pNode->data.m_size;
        pNode = pNode->pNext;
    }

    return pNode;
}

} /* namespace adt */
