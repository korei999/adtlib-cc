#pragma once

#include "IAllocator.hh"
#include "Span.hh"

#include <cstring>
#include <cassert>

namespace adt
{

/* Bump allocator, using fixed size buffer memory */
struct BufferAllocator : public IAllocator
{
    u8* m_pMemBuffer = nullptr;
    usize m_size = 0;
    usize m_cap = 0;
    void* m_pLastAlloc = nullptr;
    usize m_lastAllocSize = 0;
    
    /* */

    constexpr BufferAllocator() = default;

    constexpr BufferAllocator(u8* pMemory, usize capacity) noexcept
        : m_pMemBuffer(pMemory),
          m_cap(capacity) {}

    template<typename T, ssize N>
    BufferAllocator(T (&aMem)[N]) noexcept
        : m_pMemBuffer(static_cast<u8*>(aMem)),
          m_cap(N * sizeof(T)) {}

    template<typename T>
    constexpr BufferAllocator(Span<T> sp) noexcept
        : BufferAllocator(sp.data(), sp.getSize()) {}

    /* */

    [[nodiscard]] virtual void* malloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* zalloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, usize oldCount, usize newCount, usize mSize) noexcept(false) override final;
    constexpr virtual void free(void* ptr) noexcept override final; /* noop */
    constexpr virtual void freeAll() noexcept override final; /* same as reset */
    constexpr void reset() noexcept;
};

inline void*
BufferAllocator::malloc(usize mCount, usize mSize)
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

inline void*
BufferAllocator::zalloc(usize mCount, usize mSize)
{
    auto* p = malloc(mCount, mSize);
    memset(p, 0, align8(mCount * mSize));
    return p;
}

inline void*
BufferAllocator::realloc(void* p, usize oldCount, usize newCount, usize mSize)
{
    if (!p) return malloc(newCount, mSize);

    assert(p >= m_pMemBuffer && p < m_pMemBuffer + m_cap && "[FixedAllocator]: invalid pointer");

    usize realSize = align8(newCount * mSize);

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
        auto* ret = malloc(newCount, mSize);
        memcpy(ret, p, oldCount);

        return ret;
    }
}

constexpr void
BufferAllocator::free(void*) noexcept
{
    //
}

constexpr void
BufferAllocator::freeAll() noexcept
{
    reset();
}

constexpr void
BufferAllocator::reset() noexcept
{
    m_size = 0;
    m_pLastAlloc = nullptr;
}

} /* namespace adt */
