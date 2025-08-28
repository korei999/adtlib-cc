#pragma once

#include "adt/IAllocator.hh"
#include "adt/assert.hh"
#include "adt/utils.hh"

#if __has_include(<sys/mman.h>)
    #include <sys/mman.h>
#endif

namespace adt
{

struct Arena2 : IArena
{
    void* m_pData {};
    isize m_off {};
    isize m_reserved {};
    isize m_commited {};
    void* m_pLastAlloc {};
    isize m_lastAllocSize {};

    /* */

    Arena2(isize reserve, isize commit = getPageSize()) noexcept(false); /* AllocException */
    Arena2() noexcept = default;

    /* */

    [[nodiscard]] virtual void* malloc(usize mCount, usize mSize) noexcept(false) override; /* AllocException */

    [[nodiscard]] virtual void* zalloc(usize mCount, usize mSize) noexcept(false) override; /* AllocException */

    [[nodiscard]] virtual void* realloc(void* p, usize oldCount, usize newCount, usize mSize) noexcept(false) override; /* AllocException */

    virtual void free(void* ptr) noexcept override;

    [[nodiscard]] virtual constexpr bool doesFree() const noexcept override;
    [[nodiscard]] virtual constexpr bool doesRealloc() const noexcept override;

    virtual constexpr void freeAll() noexcept override;

    /* */

    void reset() noexcept;
    void resetToFirstPage() noexcept;

protected:
    void growIfNeeded(isize newOff);
};

inline
Arena2::Arena2(isize reserve, isize commit)
{
    [[maybe_unused]] int err = 0;

    const isize realReserved = alignUp(reserve, getPageSize());
    void* pRes = mmap(nullptr, realReserved, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (pRes == MAP_FAILED) throw AllocException{"mmap() failed"};

    m_pData = pRes;
    m_reserved = realReserved;
    m_pLastAlloc = (void*)~0lu;

    if (commit > 0)
    {
        const isize realCommit = alignUp(commit, getPageSize());

        err = mprotect(m_pData, realCommit, PROT_READ | PROT_WRITE);
        ADT_ASSERT(err != - 1, "mprotect: r: {} ({}), realCommit: {}", err, strerror(errno), realCommit);
        err = madvise(m_pData, realCommit, MADV_WILLNEED);
        ADT_ASSERT(err != - 1, "madvise: r: {} ({})", err, strerror(errno));

        m_commited = realCommit;
    }
}

inline void*
Arena2::malloc(usize mCount, usize mSize)
{
    const isize realSize = alignUp8(mCount * mSize);
    void* pRet = (void*)((u8*)m_pData + m_off);

    growIfNeeded(m_off + realSize);

    m_pLastAlloc = pRet;
    m_lastAllocSize = realSize;

    return pRet;
}

inline void*
Arena2::zalloc(usize mCount, usize mSize)
{
    void* pMem = malloc(mCount, mSize);
    ::memset(pMem, 0, mCount * mSize);
    return pMem;
}

inline void*
Arena2::realloc(void* p, usize oldCount, usize newCount, usize mSize)
{
    if (!p) return malloc(newCount, mSize);

    /* bump case */
    if (p == m_pLastAlloc)
    {
        const isize realSize = alignUp8(newCount * mSize);
        isize newOff = (m_off - m_lastAllocSize) + realSize;
        growIfNeeded(newOff);
        m_lastAllocSize = realSize;
        return p;
    }

    if (newCount <= oldCount) return p;

    void* pMem = malloc(newCount, mSize);
    if (p) ::memcpy(pMem, p, oldCount * mSize);
    return pMem;
}

inline void
Arena2::free(void*) noexcept
{
    /* noop */
}

inline constexpr bool
Arena2::doesFree() const noexcept
{
    return false;
}

inline constexpr bool
Arena2::doesRealloc() const noexcept
{
    return true;
}

inline constexpr void
Arena2::freeAll() noexcept
{
    [[maybe_unused]] int err = munmap(m_pData, m_reserved);
    ADT_ASSERT(err != - 1, "munmap: {} ({})", err, strerror(errno));

    *this = {};
}

inline void
Arena2::reset() noexcept
{
    [[maybe_unused]] int err = mprotect(m_pData, m_commited, PROT_NONE);
    ADT_ASSERT(err != - 1, "mprotect: {} ({})", err, strerror(errno));
    err = madvise(m_pData, m_commited, MADV_DONTNEED);
    ADT_ASSERT(err != - 1, "madvise: {} ({})", err, strerror(errno));

    m_off = 0;
    m_commited = 0;
    m_pLastAlloc = (void*)~0lu;
    m_lastAllocSize = 0;
}

inline void
Arena2::resetToFirstPage() noexcept
{
    const isize pageSize = getPageSize();
    [[maybe_unused]] int err = 0;

    if (m_commited > pageSize)
    {
        err = mprotect((u8*)m_pData + pageSize, m_commited - pageSize, PROT_NONE);
        ADT_ASSERT(err != - 1, "mprotect: {} ({})", err, strerror(errno));
    }
    else if (m_commited < getPageSize())
    {
        err = mprotect((u8*)m_pData + m_commited, pageSize - m_commited, PROT_READ | PROT_WRITE);
        ADT_ASSERT(err != - 1, "mprotect: {} ({})", err, strerror(errno));
    }

    m_off = 0;
    m_commited = pageSize;
    m_pLastAlloc = (void*)~0lu;
    m_lastAllocSize = 0;
}

inline void
Arena2::growIfNeeded(isize newOff)
{
    if (newOff > m_commited)
    {
        isize newCommited = utils::max((isize)alignUp(newOff, getPageSize()), m_commited * 2);
        ADT_RUNTIME_EXCEPTION_FMT(newCommited <= m_reserved, "[Arena2]: out of reserved memory, newOff: {}, m_reserved: {}", newCommited, m_reserved);

        [[maybe_unused]] int err = mprotect((u8*)m_pData + m_commited, newCommited - m_commited, PROT_READ | PROT_WRITE);
        ADT_ASSERT(err != - 1, "mprotect: r: {} ({}), newCommited: {}", err, strerror(errno), newCommited);

        m_commited = newCommited;
    }

    m_off = newOff;
}

} /* namespace adt */
