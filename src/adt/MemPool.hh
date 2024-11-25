#pragma once

#include "types.hh"
#include "defer.hh"
#include "guard.hh"

#include <cstdio>
#include <cassert>

#include <threads.h>

namespace adt
{

using MemPoolHnd = u32;

template<typename T, u32 CAP>
struct MemPool
{
    T aData[CAP];
    u32 aFreePositions[CAP];
    mtx_t mtx;
    u32 nOccupied {};
    u32 freePosIdx {};

    T& operator[](u32 i) { assert(i < CAP && i < NPOS && "[MemPool]: out of range"); return aData[i]; }
    const T& operator[](u32 i) const { assert(i < CAP && i < NPOS && "[MemPool]: out of range"); return aData[i]; }

    MemPool() = default;
    MemPool(INIT_FLAG e)
    { 
        if (e != INIT_FLAG::INIT) return;

        mtx_init(&mtx, mtx_plain);
        memset(aFreePositions, NPOS, sizeof(u32) * CAP);
    }
};

template<typename T, u32 CAP>
inline void
MemPoolDestroy(MemPool<T, CAP>* s)
{
    mtx_destroy(&s->mtx);
}

template<typename T, u32 CAP>
[[nodiscard]] inline MemPoolHnd
MemPoolRent(MemPool<T, CAP>* s)
{
    guard::Mtx lock(&s->mtx);

    if (s->nOccupied >= CAP)
    {
#ifndef NDEBUG
        fputs("[MemPool]: no free element, returning NPOS", stderr);
#endif
        return NPOS;
    }

    assert(s->freePosIdx < CAP);

    auto lastFreeIdx = s->aFreePositions[s->freePosIdx];

    if (lastFreeIdx == NPOS)
    {
        ++s->nOccupied;
        /*s->aData[s->nOccupied - 1] = {};*/
        return s->nOccupied - 1;
    }
    else
    {
        ++s->nOccupied;
        s->aFreePositions[s->freePosIdx--] = NPOS;
        /*s->aData[lastFreeIdx] = {};*/
        return lastFreeIdx;
    }
}

template<typename T, u32 CAP>
inline void
MemPoolReturn(MemPool<T, CAP>* s, MemPoolHnd idx)
{
    guard::Mtx lock(&s->mtx);

    assert(idx < CAP && idx < NPOS && "[MemPool]: returning out of range handle");
    s->aFreePositions[++s->freePosIdx] = idx;
    --s->nOccupied;
    assert(s->nOccupied < CAP && "[MemPool]: nothing to return");
}

} /* namespace adt */
