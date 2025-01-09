#pragma once

#include "IAllocator.hh"
#include "utils.hh"
#include "Span.hh"

#include <cstring>
#include <cassert>

namespace adt
{

struct FixedAllocator : public IAllocator
{
    u8* m_pMemBuffer = nullptr;
    usize m_size = 0;
    usize m_cap = 0;
    void* m_pLastAlloc = nullptr;
    usize m_lastAllocSize = 0;
    
    /* */

    constexpr FixedAllocator() = default;

    constexpr FixedAllocator(u8* pMemory, usize capacity) noexcept
        : m_pMemBuffer(pMemory),
          m_cap(capacity) {}

    template<typename T, ssize N>
    FixedAllocator(T (&aMem)[N]) noexcept
        : m_pMemBuffer(static_cast<u8*>(aMem)),
          m_cap(N * sizeof(T)) {}

    template<typename T>
    constexpr FixedAllocator(Span<T> sp) noexcept
        : FixedAllocator(sp.data(), sp.getSize()) {}

    /* */

    [[nodiscard]] virtual constexpr void* malloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual constexpr void* zalloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual constexpr void* realloc(void* ptr, usize mCount, usize mSize) noexcept(false) override final;
    constexpr virtual void free(void* ptr) noexcept override final; /* noop */
    constexpr virtual void freeAll() noexcept override final; /* same as reset */
    constexpr void reset() noexcept;
};

constexpr void*
FixedAllocator::malloc(usize mCount, usize mSize)
{
    usize realSize = align8(mCount * mSize);

    if (m_size + realSize > m_cap)
        throw AllocException("FixedAllocator::malloc(): out of memory");

    void* ret = &m_pMemBuffer[m_size];
    m_size += realSize;
    m_pLastAlloc = ret;
    m_lastAllocSize = realSize;

    return ret;
}

constexpr void*
FixedAllocator::zalloc(usize mCount, usize mSize)
{
    auto* p = malloc(mCount, mSize);
    memset(p, 0, align8(mCount * mSize));
    return p;
}

constexpr void*
FixedAllocator::realloc(void* p, usize mCount, usize mSize)
{
    if (!p) return malloc(mCount, mSize);

    assert(p >= m_pMemBuffer && p < m_pMemBuffer + m_cap && "[FixedAllocator]: invalid pointer");

    usize realSize = align8(mCount * mSize);

    if ((m_size + realSize - m_lastAllocSize) > m_cap)
        throw AllocException("FixedAllocator::realloc(): out of memory");

    if (p == m_pLastAlloc)
    {
        m_size -= m_lastAllocSize;
        m_size += realSize;
        m_lastAllocSize = realSize;

        return p;
    }
    else
    {
        auto* ret = malloc(mCount, mSize);
        usize nBytesUntilEndOfBlock = m_cap - m_size;
        usize nBytesToCopy = utils::min(realSize, nBytesUntilEndOfBlock);
        memcpy(ret, p, nBytesToCopy);

        return ret;
    }
}

constexpr void
FixedAllocator::free(void*) noexcept
{
    //
}

constexpr void
FixedAllocator::freeAll() noexcept
{
    reset();
}

constexpr void
FixedAllocator::reset() noexcept
{
    m_size = 0;
    m_pLastAlloc = nullptr;
}

} /* namespace adt */
