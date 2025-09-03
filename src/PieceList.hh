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

    using ListType = List<Piece>;
    using Node = ListType::Node;

    /* */

    ListM<Piece> m_lPieces {};
    isize m_size {};

    /* */

    PieceList() noexcept = default;

    PieceList(RefCountedPtr<StringM> rcpS);

    /* */

    isize size() const noexcept { return m_size; }
    bool empty() const noexcept { return m_lPieces.empty(); }
    void destroy() noexcept;
    Node* insert(isize pos, const StringView sv);
    Node* insert(isize pos, isize size, Node* pNode);
    void remove(isize pos, isize size);
    void defragment();

    [[nodiscard]] String toString(IAllocator* pAlloc);

    [[nodiscard]] Node* posToNode(isize* pPos) noexcept;

protected:
    Node* insertFinal(isize pos, isize size, Node* pNew);
};

inline
PieceList::PieceList(RefCountedPtr<StringM> rcpS)
{
    m_lPieces.pushBack(Piece{.m_rcpS = rcpS.ref(), .m_pos = 0, .m_size = rcpS->m_size});
    m_size = rcpS->m_size;
}

inline PieceList::Node*
PieceList::insert(isize pos, const StringView sv)
{
    Node* pNew = nullptr;
    Node* pRet = nullptr;
    RefCountedPtr<StringM> rcp {};

    try
    {
        rcp = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, sv);
        pNew = Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = rcp,
            .m_pos = 0,
            .m_size = sv.m_size,
        });
        pRet = insertFinal(pos, sv.size(), pNew);
    }
    catch (const AllocException& ex)
    {
#ifdef ADT_DBG_MEMORY
        ex.printErrorMsg(stderr);
#endif
        if (rcp) rcp.unref();
        StdAllocator::inst()->free(pNew);
    }

    return pRet;
}

inline PieceList::Node*
PieceList::insert(isize pos, isize size, Node* pNode)
{
    Node* pRet = nullptr;
    Node* pNew = nullptr;

    try
    {
        pNew = Node::alloc(StdAllocator::inst(), Piece{
            .m_rcpS = pNode->data.m_rcpS.ref(),
            .m_pos = 0,
            .m_size = size,
        });

        pRet = insertFinal(pos, size, pNew);
    }
    catch (const AllocException& ex)
    {
#ifdef ADT_DBG_MEMORY
        ex.printErrorMsg(stderr);
#endif
        pNode->data.m_rcpS.unref();
    }

    return pRet;
}

inline void
PieceList::remove(isize pos, isize size)
{
    ADT_ASSERT(!m_lPieces.empty(), "");
    ADT_ASSERT(pos >= 0 && pos < m_size && size > 0 && (pos + size) <= m_size, "m_size: {}, pos: {}, size: {}", m_size, pos, size);

    Node *pNode = posToNode(&pos);
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

    static_cast<ListType&>(m_lPieces).pushBack(Node::alloc(StdAllocator::inst(), piece));
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
PieceList::posToNode(isize* pPos) noexcept
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

inline PieceList::Node*
PieceList::insertFinal(isize pos, isize size, Node* pNew)
{
    Node* pRet;
    const isize fullSize = size;

    if (!m_lPieces.m_pFirst)
    {
        pNew->data.m_rcpS.ref();
        static_cast<ListType&>(m_lPieces).pushBack(pNew);

        m_size += fullSize;
        return pNew;
    }

    Node* pNode = posToNode(&pos);

    if (pNode->data.m_size == pos) /* append case */
    {
        m_lPieces.insertAfter(pNode, pNew);

        m_size += fullSize;
        return pNew;
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

        m_lPieces.insertAfter(pLeftNode, pNew);

        rPiece.m_size -= pos;
        rPiece.m_pos += pos;

        pRet = pNew;
    }
    else /* prepend case */
    {
        ADT_ASSERT(pos == 0, "pos: {}", pos);
        m_lPieces.insertBefore(pNode, pNew);
        pRet = pNew;
    }

    m_size += fullSize;
    return pRet;
}

} /* namespace adt */
