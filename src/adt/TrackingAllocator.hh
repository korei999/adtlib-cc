#pragma once

#include "Allocator.hh"
#include "Map.hh"
#include "OsAllocator.hh"

#include <stdlib.h>

namespace adt
{

struct TrackingAllocator;

inline void* TrackingAlloc(TrackingAllocator* s, u64 mCount, u64 mSize);
inline void* TrackingZalloc(TrackingAllocator* s, u64 mCount, u64 mSize);
inline void* TrackingRealloc(TrackingAllocator* s, void* p, u64 mCount, u64 mSize);
inline void TrackingFree(TrackingAllocator* s, void* p);
inline void TrackingFreeAll(TrackingAllocator* s);

/* OsAllocator while tracking allocations */
struct TrackingAllocator
{
    Allocator super;
    MapBase<void*> mAllocations;

    TrackingAllocator() = default;
    TrackingAllocator(u64 pre);

    [[nodiscard]] void* alloc(u64 mCount, u64 mSize) { return TrackingAlloc(this, mCount, mSize); }
    [[nodiscard]] void* zalloc(u64 mCount, u64 mSize) { return TrackingZalloc(this, mCount, mSize); }
    [[nodiscard]] void* realloc(void* ptr, u64 mCount, u64 mSize) { return TrackingRealloc(this, ptr, mCount, mSize); }
    void free(void* ptr) { TrackingFree(this, ptr); }
    void freeAll() { TrackingFreeAll(this); }
};

inline void*
TrackingAlloc(TrackingAllocator* s, u64 mCount, u64 mSize)
{
    void* r = ::malloc(mCount * mSize);
    MapInsert(&s->mAllocations, &s->super, r);
    return r;
}

inline void*
TrackingZalloc(TrackingAllocator* s, u64 mCount, u64 mSize)
{
    void* r = ::calloc(mCount, mSize);
    MapInsert(&s->mAllocations, &s->super, r);
    return r;
}

inline void*
TrackingRealloc(TrackingAllocator* s, void* p, u64 mCount, u64 mSize)
{
    void* r = ::realloc(p, mCount * mSize);

    if (p != r)
    {
        MapRemove(&s->mAllocations, p);
        MapInsert(&s->mAllocations, &s->super, r);
    }

    return r;
}

inline void
TrackingFree(TrackingAllocator* s, void* p)
{
    MapRemove(&s->mAllocations, p);
    ::free(p);
}

inline void
TrackingFreeAll(TrackingAllocator* s)
{
    for (auto& b : s->mAllocations)
        ::free(b);

    MapDestroy(&s->mAllocations, &s->super);
}

inline const AllocatorVTable inl_TrackingAllocatorVTable {
    .alloc = decltype(AllocatorVTable::alloc)(+[](TrackingAllocator* s, u64 mCount, u64 mSize) { return s->alloc(mCount, mSize); }),
    .zalloc = decltype(AllocatorVTable::zalloc)(+[](TrackingAllocator* s, u64 mCount, u64 mSize) { return s->zalloc(mCount, mSize); }),
    .realloc = decltype(AllocatorVTable::realloc)(+[](TrackingAllocator* s, void* ptr, u64 mCount, u64 mSize) { return s->realloc(ptr, mCount, mSize); }),
    .free = decltype(AllocatorVTable::free)(+[](TrackingAllocator* s, void* ptr) { s->free(ptr); }),
    .freeAll = decltype(AllocatorVTable::freeAll)(+[](TrackingAllocator* s) { s->freeAll(); } ),
};

inline
TrackingAllocator::TrackingAllocator(u64 pre)
    : super {&inl_TrackingAllocatorVTable}, mAllocations(&inl_OsAllocator.super, pre * 2) {}

} /* namespace adt */
