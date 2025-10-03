#pragma once

#include "ArenaDeleterCRTP.hh"

namespace adt
{

struct BufferAllocatorState;

/* Bump allocator, using fixed size memory buffer. */
struct BufferAllocator : public IArena, public ArenaDeleterCRTP<BufferAllocator>
{
    using Deleter = ArenaDeleterCRTP<BufferAllocator>;

    /* */

    u8* m_pMemBuffer = nullptr;
    usize m_size = 0;
    usize m_cap = 0;
    void* m_pLastAlloc = nullptr;
    usize m_lastAllocSize = 0;

    /* */

    BufferAllocator() = default;

    BufferAllocator(u8* pMemory, usize capacity) noexcept
        : Deleter{INIT},
          m_pMemBuffer{pMemory},
          m_cap{capacity}
    {}

    template<typename T, usize N>
    BufferAllocator(T (&aMem)[N]) noexcept
        : Deleter{INIT},
          m_pMemBuffer{reinterpret_cast<u8*>(aMem)},
          m_cap{N * sizeof(T)}
    {}

    template<typename T>
    BufferAllocator(Span<T> sp) noexcept
        : BufferAllocator{reinterpret_cast<u8*>(sp.data()), sp.size() * sizeof(T)}
    {}

    /* */

    [[nodiscard]] virtual void* malloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* zalloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, usize oldCount, usize newCount, usize mSize) noexcept(false) override final;
    [[deprecated("noop")]] virtual void free(void*) noexcept override final { /* noop */ }
    virtual void freeAll() noexcept override final; /* same as reset */
    [[nodiscard]] virtual bool doesFree() const noexcept override final { return false; }
    [[nodiscard]] virtual bool doesRealloc() const noexcept override final { return true; }

    /* */

    void reset() noexcept;
    usize realCap() const noexcept;
    usize memoryUsed() const noexcept;
};

struct BufferAllocatorState
{
    usize m_size {};
    void* m_pLastAlloc {};
    usize m_lastAllocSize {};

    /* */

    void restore(BufferAllocator* pAlloc) noexcept;
};

struct BufferAllocatorScope
{
    BufferAllocator* m_pAlloc {};
    BufferAllocatorState m_offsets {};

    /* */

    BufferAllocatorScope(BufferAllocator* pAlloc) noexcept;
    ~BufferAllocatorScope() noexcept;
};

inline void
BufferAllocatorState::restore(BufferAllocator* pAlloc) noexcept
{
    pAlloc->runDeleters();

    pAlloc->m_size = m_size;
    pAlloc->m_pLastAlloc = m_pLastAlloc;
    pAlloc->m_lastAllocSize = m_lastAllocSize;
}

inline
BufferAllocatorScope::BufferAllocatorScope(BufferAllocator* pAlloc) noexcept
    : m_pAlloc{pAlloc}, m_offsets{.m_size = pAlloc->m_size, .m_pLastAlloc = pAlloc->m_pLastAlloc, .m_lastAllocSize = pAlloc->m_lastAllocSize} {}

inline
BufferAllocatorScope::~BufferAllocatorScope() noexcept
{
    m_offsets.restore(m_pAlloc);
}

inline void*
BufferAllocator::malloc(usize mCount, usize mSize)
{
    usize realSize = alignUp8PO2(mCount * mSize);

    if (m_size + realSize > m_cap)
    {
        errno = ENOBUFS;
        throw AllocException("BufferAllocator::malloc(): out of memory");
    }

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
    memset(p, 0, mCount*mSize);
    return p;
}

inline void*
BufferAllocator::realloc(void* p, usize oldCount, usize newCount, usize mSize)
{
    if (!p) return malloc(newCount, mSize);

    ADT_ASSERT(p >= m_pMemBuffer && p < m_pMemBuffer + m_cap, "invalid pointer");

    const usize realSize = alignUp8PO2(newCount * mSize);

    if ((m_size + realSize - m_lastAllocSize) > m_cap)
    {
        errno = ENOBUFS;
        throw AllocException("BufferAllocator::realloc(): out of memory");
    }

    if (p == m_pLastAlloc)
    {
        m_size -= m_lastAllocSize;
        m_size += realSize;
        m_lastAllocSize = realSize;

        return p;
    }
    else
    {
        if (newCount <= oldCount) return p;

        auto* ret = malloc(newCount, mSize);
        memcpy(ret, p, oldCount);

        return ret;
    }
}

inline void
BufferAllocator::freeAll() noexcept
{
    reset();
}

inline void
BufferAllocator::reset() noexcept
{
    runDeleters();
    m_size = 0;
    m_pLastAlloc = nullptr;
}

inline usize
BufferAllocator::realCap() const noexcept
{
    return alignDown8PO2(m_cap);
}

inline usize
BufferAllocator::memoryUsed() const noexcept
{
    return m_size;
}

} /* namespace adt */
