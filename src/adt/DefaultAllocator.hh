#pragma once

#include "Allocator.hh"

#include <stdlib.h>

namespace adt
{

struct DefaultAllocator;

inline void* DefaultAlloc(DefaultAllocator* s, u64 mCount, u64 mSize);
inline void* DefaultRealloc(DefaultAllocator* s, void* p, u64 mCount, u64 mSize);
inline void DefaultFree(DefaultAllocator* s, void* p);

struct DefaultAllocator
{
    Allocator base {};

    DefaultAllocator()
    {
        static const Allocator::Interface vTable {
            .alloc = (decltype(Allocator::Interface::alloc))DefaultAlloc,
            .realloc = (decltype(Allocator::Interface::realloc))DefaultRealloc,
            .free = (decltype(Allocator::Interface::free))DefaultFree
        };

        this->base = {&vTable};
    }
};

inline void*
DefaultAlloc([[maybe_unused]] DefaultAllocator* s, u64 mCount, u64 mSize)
{
    return ::calloc(mCount, mSize);
}

inline void*
DefaultRealloc([[maybe_unused]] DefaultAllocator* s, void* p, u64 mCount, u64 mSize)
{
    return ::reallocarray(p, mCount, mSize);
}

inline void
DefaultFree([[maybe_unused]] DefaultAllocator* s, void* p)
{
    ::free(p);
}


} /* namespace adt */
