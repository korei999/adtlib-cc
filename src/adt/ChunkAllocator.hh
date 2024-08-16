#pragma once

#include "Allocator.hh"

namespace adt
{

/* each alloc is the same size (good for linked nodes allocation) */
struct ChunkAllocator
{
    Allocator base {};
    u8* pMemBuffer = nullptr;
};

} /* namespace adt */
