#pragma once

#include "adt/IAllocator.hh"
#include "adt/assert.hh"

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
    const isize realReserved = alignUp(reserve, getPageSize());
    void* pRes = mmap(nullptr, realReserved, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (pRes == nullptr) throw AllocException{"mmap() failed"};

    m_pData = pRes;
    m_reserved = realReserved;
    m_pLastAlloc = (void*)~0lu;

    if (commit > 0)
    {
        const isize realCommit = alignUp(commit, getPageSize());

        int r = mprotect(m_pData, realCommit, PROT_READ | PROT_WRITE);
        ADT_RUNTIME_EXCEPTION_FMT(r != - 1, "mprotect: r: {} ({}), realCommit: {}", r, strerror(errno), realCommit);

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
    [[maybe_unused]] int r = munmap(m_pData, m_reserved);
    ADT_ASSERT(r != - 1, "munmap: {} ({})", r, strerror(errno));

    *this = {};
}

inline void
Arena2::reset() noexcept
{
    [[maybe_unused]] int r = mprotect(m_pData, m_commited, PROT_NONE);
    ADT_ASSERT(r != - 1, "mprotect: {} ({})", r, strerror(errno));
    r = madvise(m_pData, m_commited, MADV_DONTNEED);
    ADT_ASSERT(r != - 1, "madvise: {} ({})", r, strerror(errno));

    m_off = 0;
    m_commited = 0;
    m_pLastAlloc = (void*)~0lu;
    m_lastAllocSize = 0;
}

inline void
Arena2::resetToFirstPage() noexcept
{
    // const isize prevCommited = m_commited;

    [[maybe_unused]] int r = mprotect(m_pData, getPageSize(), PROT_READ | PROT_WRITE);
    ADT_ASSERT(r != - 1, "mprotect: {} ({})", r, strerror(errno));

    m_off = 0;
    m_commited = getPageSize();
    m_pLastAlloc = (void*)~0lu;
    m_lastAllocSize = 0;
}

inline void
Arena2::growIfNeeded(isize newOff)
{
    if (newOff > m_commited)
    {
        ADT_RUNTIME_EXCEPTION_FMT(newOff <= m_reserved, "newOff: {}, m_reserved: {}", newOff, m_reserved);
        isize newCommited = alignUp(newOff, getPageSize());
        int r = mprotect(m_pData, newCommited, PROT_READ | PROT_WRITE);
        ADT_RUNTIME_EXCEPTION_FMT(r != - 1, "mprotect: r: {} ({}), newCommited: {}", r, strerror(errno), newCommited);
        m_commited = newCommited;
    }

    m_off = newOff;
}

} /* namespace adt */
