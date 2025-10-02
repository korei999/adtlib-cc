#pragma once

#include "SList.hh"

#include <cstring>

namespace adt
{

struct ArenaListScope;

/* Like Arena, but uses list chained memory blocks allocated with backing allocator. */
struct ArenaList : public IArena
{
    friend ArenaListScope;

    struct Block
    {
        Block* pNext {};
        usize size {}; /* excluding sizeof(ArenaBlock) */
        usize nBytesOccupied {};
        u8* pLastAlloc {};
        usize lastAllocSize {};
        u8 pMem[];
    };

    using PfnDeleter = void(*)(void**);

    struct DeleterNode
    {
        void** ppObj {};
        PfnDeleter pfnDelete {};
    };

    using ListNodeType = SList<DeleterNode>::Node;

    /* */

    usize m_defaultCapacity {};
    IAllocator* m_pBackAlloc {};
#ifndef NDEBUG
    std::source_location m_loc {};
#endif
    Block* m_pBlocks {};
    SList<DeleterNode> m_lDeleters {}; /* Run deleters on reset()/freeAll() or state restorations. */
    SList<DeleterNode>* m_pLCurrentDeleters {};

    /* */

    ArenaList() = default;

    ArenaList(
        usize capacity,
        IAllocator* pBackingAlloc = Gpa::inst()
#ifndef NDEBUG
        , std::source_location _DONT_USE_loc = std::source_location::current()
#endif
    ) noexcept(false)
        : m_defaultCapacity(alignUp8(capacity)),
          m_pBackAlloc(pBackingAlloc),
#ifndef NDEBUG
          m_loc {_DONT_USE_loc},
#endif
          m_pBlocks(allocBlock(m_defaultCapacity)),
          m_pLCurrentDeleters{&m_lDeleters}
    {}

    /* */

    [[nodiscard]] virtual void* malloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* zalloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, usize oldCount, usize newCount, usize mSize) noexcept(false) override final;
    virtual void free(void* ptr) noexcept override final;
    virtual void freeAll() noexcept override final;
    [[nodiscard]] virtual constexpr bool doesFree() const noexcept override final { return false; }
    [[nodiscard]] virtual constexpr bool doesRealloc() const noexcept override final { return true; }

    /* */

    void reset() noexcept;
    void shrinkToFirstBlock() noexcept;
    isize memoryUsed() const noexcept;

    /* */

    template<typename T>
    struct Ptr : protected ListNodeType
    {
        T* m_pData {};

        /* */

        Ptr() noexcept = default;

        template<typename ...ARGS>
        Ptr(ArenaList* pArena, ARGS&&... args)
            : ListNodeType{nullptr, {(void**)this, (PfnDeleter)nullptrDeleter}},
              m_pData {pArena->alloc<T>(std::forward<ARGS>(args)...)}
        {
            pArena->m_pLCurrentDeleters->insert(static_cast<ListNodeType*>(this));
        }

        template<typename ...ARGS>
        Ptr(void (*pfn)(Ptr*), ArenaList* pArena, ARGS&&... args)
            : ListNodeType{nullptr, {(void**)this, (PfnDeleter)pfn}},
              m_pData {pArena->alloc<T>(std::forward<ARGS>(args)...)}
        {
            pArena->m_pLCurrentDeleters->insert(static_cast<ListNodeType*>(this));
        }

        /* */

        static void
        nullptrDeleter(Ptr* pPtr) noexcept
        {
            utils::destruct(pPtr->m_pData);
            pPtr->m_pData = nullptr;
        };

        static void
        simpleDeleter(Ptr* pPtr) noexcept
        {
            utils::destruct(pPtr->m_pData);
        };

        /* */

        explicit operator bool() const noexcept { return m_pData != nullptr; }

        T& operator*() noexcept { ADT_ASSERT(m_pData != nullptr, ""); return *m_pData; }
        const T& operator*() const noexcept { ADT_ASSERT(m_pData != nullptr, ""); return *m_pData; }

        T* operator->() noexcept { ADT_ASSERT(m_pData != nullptr, ""); return m_pData; }
        const T* operator->() const noexcept { ADT_ASSERT(m_pData != nullptr, ""); return m_pData; }
    };

protected:
    [[nodiscard]] inline Block* allocBlock(usize size);
    [[nodiscard]] inline Block* prependBlock(usize size);
    [[nodiscard]] inline Block* findFittingBlock(usize size);
    [[nodiscard]] inline Block* findBlockFromPtr(u8* ptr);

    ADT_NO_UB void runDeleters() noexcept;
};

/* We track only last block, and free() all new blocks if they were created. */
struct ArenaListState
{
    ArenaList* m_pArena {};
    ArenaList::Block* m_pLastBlock {};
    usize m_nBytesOccupied {};
    u8* m_pLastAlloc {};
    usize m_lastAllocSize {};

    /* */

    ArenaListState() = default;
    ArenaListState(ArenaList* pArena) noexcept;

    /* */

    void restore() noexcept;
};

struct ArenaListScope
{
    ArenaListState m_state {};
    SList<ArenaList::DeleterNode> m_lDeleters {};

    /* */

    ArenaListScope(ArenaList* p) noexcept : m_state{p} {}

    ~ArenaListScope() noexcept;
};

inline
ArenaListScope::~ArenaListScope() noexcept
{
    m_state.m_pArena->runDeleters();
    m_state.restore();
}

inline
ArenaListState::ArenaListState(ArenaList* pArena) noexcept
    : m_pArena{pArena},
      m_pLastBlock{pArena->m_pBlocks},
      m_nBytesOccupied{m_pLastBlock->nBytesOccupied},
      m_pLastAlloc{m_pLastBlock->pLastAlloc},
      m_lastAllocSize{m_pLastBlock->lastAllocSize}
{
}

inline void
ArenaListState::restore() noexcept
{
    m_pLastBlock->nBytesOccupied = m_nBytesOccupied;
    m_pLastBlock->pLastAlloc = m_pLastAlloc;
    m_pLastBlock->lastAllocSize = m_lastAllocSize;

    auto* it = m_pArena->m_pBlocks;
    while (it != m_pLastBlock)
    {
#if defined ADT_DBG_MEMORY && !defined NDEBUG
        LogDebug("[Arena: {}, {}, {}]: deallocating block of size {} on state restoration\n",
            print::shorterSourcePath(m_pArena->m_loc.file_name()),
            m_pArena->m_loc.function_name(),
            m_pArena->m_loc.line(),
            it->size
        );
#endif
        auto* next = it->pNext;
        m_pArena->m_pBackAlloc->free(it);
        it = next;
    }

    m_pLastBlock->pNext = nullptr;
    m_pArena->m_pBlocks = m_pLastBlock;
}

inline ArenaList::Block*
ArenaList::findBlockFromPtr(u8* ptr)
{
    auto* it = m_pBlocks;
    while (it)
    {
        if (ptr >= it->pMem && ptr < &it->pMem[it->size])
            return it;

        it = it->pNext;
    }

    return nullptr;
}

inline void
ArenaList::runDeleters() noexcept
{
    for (auto e : *m_pLCurrentDeleters)
        e.pfnDelete(e.ppObj);

    m_pLCurrentDeleters->m_pHead = nullptr;
}

inline ArenaList::Block*
ArenaList::findFittingBlock(usize size)
{
    auto* it = m_pBlocks;
    while (it)
    {
        if (size < it->size - it->nBytesOccupied)
            return it;

        it = it->pNext;
    }

    return nullptr;
}

inline ArenaList::Block*
ArenaList::allocBlock(usize size)
{
    ADT_ASSERT(m_pBackAlloc, "uninitialized: m_pBackAlloc == nullptr");

    /* NOTE: m_pBackAlloc can throw here */
    Block* pBlock = static_cast<Block*>(m_pBackAlloc->zalloc(1, size + sizeof(*pBlock)));

#if defined ADT_DBG_MEMORY && !defined NDEBUG
    LogDebug("[Arena: {}, {}, {}]: new block of size: {}\n",
        print::shorterSourcePath(m_loc.file_name()), m_loc.function_name(), m_loc.line(), size
    );
#endif

    pBlock->size = size;
    pBlock->pLastAlloc = pBlock->pMem;

    return pBlock;
}

inline ArenaList::Block*
ArenaList::prependBlock(usize size)
{
    auto* pNew = allocBlock(size);
    pNew->pNext = m_pBlocks;
    m_pBlocks = pNew;

    return pNew;
}

inline void*
ArenaList::malloc(usize mCount, usize mSize)
{
    usize realSize = alignUp8(mCount * mSize);
    auto* pBlock = findFittingBlock(realSize);

#if defined ADT_DBG_MEMORY && !defined NDEBUG
    if (m_defaultCapacity <= realSize)
        LogDebug("[Arena: {}, {}, {}]: allocating more than defaultCapacity ({}, {})\n",
            print::shorterSourcePath(m_loc.file_name()), m_loc.function_name(), m_loc.line(), m_defaultCapacity, realSize
        );
#endif

    if (!pBlock) pBlock = prependBlock(utils::max(m_defaultCapacity, usize(realSize*1.33)));

    auto* pRet = pBlock->pMem + pBlock->nBytesOccupied;
    ADT_ASSERT(pRet == pBlock->pLastAlloc + pBlock->lastAllocSize, " ");

    pBlock->nBytesOccupied += realSize;
    pBlock->pLastAlloc = pRet;
    pBlock->lastAllocSize = realSize;

    return pRet;
}

inline void*
ArenaList::zalloc(usize mCount, usize mSize)
{
    auto* p = malloc(mCount, mSize);
    memset(p, 0, alignUp8(mCount * mSize));
    return p;
}

inline void*
ArenaList::realloc(void* ptr, usize oldCount, usize mCount, usize mSize)
{
    if (!ptr) return malloc(mCount, mSize);

    const usize requested = mSize * mCount;
    const usize realSize = alignUp8(requested);

    auto* pBlock = findBlockFromPtr(static_cast<u8*>(ptr));
    if (!pBlock) throw AllocException("pointer doesn't belong to this arena");

    if (ptr == pBlock->pLastAlloc &&
        pBlock->pLastAlloc + realSize < pBlock->pMem + pBlock->size) /* bump case */
    {
        pBlock->nBytesOccupied -= pBlock->lastAllocSize;
        pBlock->nBytesOccupied += realSize;
        pBlock->lastAllocSize = realSize;

        return ptr;
    }
    else
    {
        if (mCount <= oldCount) return ptr;

        auto* pRet = malloc(mCount, mSize);
        memcpy(pRet, ptr, oldCount * mSize);

        return pRet;
    }
}

inline void
ArenaList::free(void*) noexcept
{
    /* noop */
}

inline void
ArenaList::freeAll() noexcept
{
    auto* it = m_pBlocks;
    while (it)
    {
        auto* next = it->pNext;
        m_pBackAlloc->free(it);
        it = next;
    }
    m_pBlocks = nullptr;
}

inline void
ArenaList::reset() noexcept
{
    auto* it = m_pBlocks;
    while (it)
    {
        it->nBytesOccupied = 0;
        it->lastAllocSize = 0;
        it->pLastAlloc = it->pMem;

        it = it->pNext;
    }
}

inline void
ArenaList::shrinkToFirstBlock() noexcept
{
    auto* it = m_pBlocks;
    if (!it) return;

    while (it->pNext)
    {
#if defined ADT_DBG_MEMORY && !defined NDEBUG
        LogDebug("[Arena: {}, {}, {}]: shrinking {} sized block\n",
            print::shorterSourcePath(m_loc.file_name()), m_loc.function_name(), m_loc.line(), it->size
        );
#endif
        auto* next = it->pNext;
        m_pBackAlloc->free(it);
        it = next;
    }
    m_pBlocks = it;
}

inline isize
ArenaList::memoryUsed() const noexcept
{
    isize total = 0;
    auto* it = m_pBlocks;
    while (it)
    {
        total += it->nBytesOccupied;
        it = it->pNext;
    }

    return total;
}

} /* namespace adt */
