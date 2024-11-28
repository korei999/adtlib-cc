#pragma once

#include "Arena.hh"
#include "guard.hh"

namespace adt
{

struct MutexArena;

inline void* MutexArenaAlloc(MutexArena* s, u64 mCount, u64 mSize);
inline void* MutexArenaZalloc(MutexArena* s, u64 mCount, u64 mSize);
inline void* MutexArenaRealloc(MutexArena* s, void* p, u64 mCount, u64 mSize);
inline void _MutexArenaFree([[maybe_unused]] MutexArena* s, [[maybe_unused]] void* p);
inline void MutexArenaFreeAll(MutexArena* s);
inline void MutexArenaReset(MutexArena* s);

struct MutexArena
{
    Arena base;
    mtx_t mtx;

    MutexArena() = default;
    MutexArena(u32 blockCap);

    [[nodiscard]] void* alloc(u64 mCount, u64 mSize) { return MutexArenaAlloc(this, mCount, mSize); }
    [[nodiscard]] void* zalloc(u64 mCount, u64 mSize) { return MutexArenaZalloc(this, mCount, mSize); }
    [[nodiscard]] void* realloc(void* ptr, u64 mCount, u64 mSize) { return MutexArenaRealloc(this, ptr, mCount, mSize); }
    void free(void* ptr) { _MutexArenaFree(this, ptr); }
    void freeAll() { MutexArenaFreeAll(this); }
    void reset() { MutexArenaReset(this); }
};

inline void*
MutexArenaAlloc(MutexArena* s, u64 mCount, u64 mSize)
{
    guard::Mtx lock(&s->mtx);
    return ArenaAlloc(&s->base, mCount, mSize);
}

inline void*
MutexArenaZalloc(MutexArena* s, u64 mCount, u64 mSize)
{
    guard::Mtx lock(&s->mtx);
    return ArenaZalloc(&s->base, mCount, mSize);
}

inline void*
MutexArenaRealloc(MutexArena* s, void* p, u64 mCount, u64 mSize)
{
    guard::Mtx lock(&s->mtx);
    return ArenaRealloc(&s->base, p, mCount, mSize);
}

inline void
_MutexArenaFree([[maybe_unused]] MutexArena* s, [[maybe_unused]] void* p)
{
    /* no individual frees */
}

inline void
MutexArenaFreeAll(MutexArena* s)
{
    guard::Mtx lock(&s->mtx);
    ArenaFreeAll(&s->base);
}

inline void
MutexArenaReset(MutexArena* s)
{
    guard::Mtx lock(&s->mtx);
    ArenaReset(&s->base);
}

inline const AllocatorVTable __AtomicArenaAllocatorVTable {
    .alloc = decltype(AllocatorVTable::alloc)(+[](MutexArena* s, u64 mCount, u64 mSize) { return s->alloc(mCount, mSize); }),
    .zalloc = decltype(AllocatorVTable::zalloc)(+[](MutexArena* s, u64 mCount, u64 mSize) { return s->zalloc(mCount, mSize); }),
    .realloc = decltype(AllocatorVTable::realloc)(+[](MutexArena* s, void* ptr, u64 mCount, u64 mSize) { return s->realloc(ptr, mCount, mSize); }),
    .free = decltype(AllocatorVTable::free)(+[](MutexArena* s, void* ptr) { s->free(ptr); }),
    .freeAll = decltype(AllocatorVTable::freeAll)(+[](MutexArena* s) { s->freeAll(); } ),
};

inline
MutexArena::MutexArena(u32 blockCap)
    : base(blockCap)
{
    base.super = {&__AtomicArenaAllocatorVTable};
    mtx_init(&mtx, mtx_plain);
}

inline void* alloc(MutexArena* s, u64 mCount, u64 mSize) { return MutexArenaAlloc(s, mCount, mSize); }
inline void* zalloc(MutexArena* s, u64 mCount, u64 mSize) { return MutexArenaZalloc(s, mCount, mSize); }
inline void* realloc(MutexArena* s, void* p, u64 mCount, u64 mSize) { return MutexArenaRealloc(s, p, mCount, mSize); }
inline void free(MutexArena* s, void* p) { _MutexArenaFree(s, p); }
inline void freeAll(MutexArena* s) { MutexArenaFreeAll(s); }

} /* namespace adt */
