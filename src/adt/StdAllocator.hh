#pragma once

#include "IAllocator.hh"
#include "assert.hh"
#include "print.hh" /* IWYU pragma: keep */

#include <cstdlib>

#ifdef ADT_USE_MIMALLOC
    #include "mimalloc.h"
#endif

namespace adt
{

struct StdAllocatorNV
{
    IAllocator* operator&() const;

    [[nodiscard]] static void* malloc(usize mCount, usize mSize) noexcept(false);
    [[nodiscard]] static void* zalloc(usize mCount, usize mSize) noexcept(false);
    [[nodiscard]] static void* realloc(void* ptr, usize oldCount, usize newCount, usize mSize) noexcept(false);
    static void free(void* ptr) noexcept;
};

/* default os allocator (aka malloc() / calloc() / realloc() / free()).
 * freeAll() method is not supported. */
struct StdAllocator : IAllocator
{
    static StdAllocator* inst();

    /* */

    [[nodiscard]] virtual void* malloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* zalloc(usize mCount, usize mSize) noexcept(false) override final;
    [[nodiscard]] virtual void* realloc(void* ptr, usize oldCount, usize newCount, usize mSize) noexcept(false) override final;
    void virtual free(void* ptr) noexcept override final;
    ADT_WARN_LEAK void virtual freeAll() noexcept override final; /* assert(false) */
};

inline IAllocator*
StdAllocatorNV::operator&() const
{
    return StdAllocator::inst();
}

inline void*
StdAllocatorNV::malloc(usize mCount, usize mSize)
{
#ifdef ADT_USE_MIMALLOC
    auto* r = ::mi_malloc(mCount * mSize);
#else
    auto* r = ::malloc(mCount * mSize);
#endif

    if (!r) throw AllocException("StdAllocatorNV::malloc()");

    return r;
}

inline void*
StdAllocatorNV::zalloc(usize mCount, usize mSize)
{
#ifdef ADT_USE_MIMALLOC
    auto* r = ::mi_zalloc(mCount * mSize);
#else
    auto* r = ::calloc(mCount, mSize);
#endif

    if (!r) throw AllocException("StdAllocatorNV::zalloc()");

    return r;
}

inline void*
StdAllocatorNV::realloc(void* p, usize, usize newCount, usize mSize)
{
#ifdef ADT_USE_MIMALLOC
    auto* r = ::mi_realloc(p, newCount * mSize);
#else
    auto* r = ::realloc(p, newCount * mSize);
#endif

    if (!r) throw AllocException("StdAllocatorNV::realloc()");

    return r;
}

inline void
StdAllocatorNV::free(void* p) noexcept
{
#ifdef ADT_USE_MIMALLOC
    ::mi_free(p);
#else
    ::free(p);
#endif
}

inline StdAllocator*
StdAllocator::inst()
{
    static StdAllocator s_instance {};
    return &s_instance;
}

inline void*
StdAllocator::malloc(usize mCount, usize mSize)
{
    return StdAllocatorNV::malloc(mCount, mSize);
}

inline void*
StdAllocator::zalloc(usize mCount, usize mSize)
{
    return StdAllocatorNV::zalloc(mCount, mSize);
}

inline void*
StdAllocator::realloc(void* ptr, usize oldCount, usize newCount, usize mSize)
{
    return StdAllocatorNV::realloc(ptr, oldCount, newCount, mSize);
}

inline void
StdAllocator::free(void* ptr) noexcept
{
    return StdAllocatorNV::free(ptr);
}

inline void
StdAllocator::freeAll() noexcept
{
    ADT_ASSERT(false, "no 'freeAll()' method");
}

struct StdAllocatorPmr : StdAllocator
{
    StdAllocatorPmr(StdAllocatorNV)
    {
    }
};

} /* namespace adt */
