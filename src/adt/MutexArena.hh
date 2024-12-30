#pragma once

#include <threads.h>

#include "Arena.hh"

namespace adt
{

struct MutexArena : IAllocator
{
    Arena m_arena {};
    mtx_t m_mtx {};

    /* */

    MutexArena() = default;
    MutexArena(IAllocator* pBackAlloc, u32 blockCap)
        : m_arena(pBackAlloc, blockCap)
    {
        mtx_init(&m_mtx, mtx_plain);
    }

    /* */

    [[nodiscard]] virtual void* malloc(u64 mCount, u64 mSize) override final;
    [[nodiscard]] virtual void* zalloc(u64 mCount, u64 mSize) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, u64 mCount, u64 mSize) override final;
    virtual void free(void* ptr) override final;
    virtual void freeAll() override final;
};

inline void*
MutexArena::malloc(u64 mCount, u64 mSize)
{
    mtx_lock(&m_mtx);
    auto* r = m_arena.malloc(mCount, mSize);
    mtx_unlock(&m_mtx);

    return r;
}

inline void*
MutexArena::zalloc(u64 mCount, u64 mSize)
{
    mtx_lock(&m_mtx);
    auto* r = m_arena.zalloc(mCount, mSize);
    mtx_unlock(&m_mtx);

    return r;
}

inline void*
MutexArena::realloc(void* p, u64 mCount, u64 mSize)
{
    mtx_lock(&m_mtx);
    auto* r = m_arena.realloc(p, mCount, mSize);
    mtx_unlock(&m_mtx);

    return r;
}

inline void
MutexArena::free(void*)
{
    /* noop */
}

inline void
MutexArena::freeAll()
{
    m_arena.freeAll();
    mtx_destroy(&m_mtx);
}

} /* namespace adt */
