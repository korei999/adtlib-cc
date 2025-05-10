#pragma once

#include "types.hh"

#include <cstdlib>

namespace rdt
{

/* TODO: virtual allocator interface. */
struct IAllocator
{
};

struct StdAllocator
{
    template<typename T>
    T* mallocV(ssize nMembers)
    {
        void* p = malloc(nMembers * sizeof(T));
        return static_cast<T*>(p);
    }

    template<typename T>
    T* callocV(ssize nMembers)
    {
        void* p = calloc(nMembers, sizeof(T));
        return static_cast<T*>(p);
    }

    void* malloc(ssize nBytes) const;
    void* calloc(ssize nMembers, ssize memberSize) const;
    void free(void* p) const noexcept;
};

inline void*
StdAllocator::malloc(ssize nBytes) const
{
    return ::malloc(nBytes);
}

inline void*
StdAllocator::calloc(ssize nMembers, ssize memberSize) const
{
    return ::calloc(nMembers, memberSize);
}

inline void
StdAllocator::free(void* p) const noexcept
{
    ::free(p);
}

} /* namespace rdt */
