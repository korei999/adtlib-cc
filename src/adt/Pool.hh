#pragma once

#include "types.hh"
#include "guard.hh"
#include "Arr.hh"

#include <cstdio>
#include <cassert>
#include <limits>

#include <threads.h>
// #include <stdatomic.h>

namespace adt
{

using PoolHnd = u32;

template<typename T>
struct PoolNode
{
    T data {};
    int nRefs {}; /* doesn't have to be atomic */
    bool bDeleted {};
};

template<typename T, u32 CAP> struct Pool;

template<typename T, u32 CAP> inline s64 PoolNextIdx(Pool<T, CAP>* s, s64 i);
template<typename T, u32 CAP> inline s64 PoolPrevIdx(Pool<T, CAP>* s, s64 i);
template<typename T, u32 CAP> inline s64 PoolFirstIdx(Pool<T, CAP>* s);
template<typename T, u32 CAP> inline s64 PoolLastIdx(Pool<T, CAP>* s);

/* statically allocated reusable resource collection */
template<typename T, u32 CAP>
struct Pool
{
    Arr<PoolNode<T>, CAP> aNodes {};
    Arr<PoolHnd, CAP> aFreeIdxs {};
    u32 nOccupied {};
    mtx_t mtx;
    mtx_t mtxRef;

    T& operator[](s64 i) { assert(!aNodes[i].bDeleted && "[MemPool]: accessing deleted node"); return aNodes[i].data; }
    const T& operator[](s64 i) const { assert(!aNodes[i].bDeleted && "[MemPool]: accessing deleted node"); return aNodes[i].data; }

    Pool() = default;
    Pool(INIT_FLAG e)
    { 
        if (e != INIT_FLAG::INIT) return;

        mtx_init(&mtx, mtx_recursive);
        mtx_init(&mtx, mtx_plain);
        for (auto& e : this->aNodes) e.bDeleted = true;
    }

    struct It
    {
        Pool* s {};
        s64 i {};

        It(Pool* _self, s64 _i) : s(_self), i(_i) {}

        T& operator*() { return s->aNodes[i].data; }
        T* operator->() { return &s->aNodes[i].data; }

        It
        operator++()
        {
            i = PoolNextIdx(s, i);
            return {s, i};
        }

        It
        operator++(int)
        {
            s64 tmp = i;
            i = PoolNextIdx(s, i);
            return {s, tmp};
        }

        It
        operator--()
        {
            i = PoolPrevIdx(s, i);
            return {s, i};
        }

        It
        operator--(int)
        {
            s64 tmp = i;
            i = PoolPrevIdx(s, i);
            return {s, tmp};
        }

        friend bool operator==(It l, It r) { return l.i == r.i; }
        friend bool operator!=(It l, It r) { return l.i != r.i; }
    };

    It begin() { return {this, PoolFirstIdx(this)}; }
    It end() { return {this, PoolLastIdx(this) + 1}; }
    It rbegin() { return {this, PoolLastIdx(this)}; }
    It rend() { return {this, PoolFirstIdx(this) - 1}; }

    const It begin() const { return {this, PoolFirstIdx(this)}; }
    const It end() const { return {this, PoolLastIdx(this) + 1}; }
    const It rbegin() const { return {this, PoolLastIdx(this)}; }
    const It rend() const { return {this, PoolFirstIdx(this) - 1}; }
};

template<typename T, u32 CAP>
inline s64
PoolFirstIdx(Pool<T, CAP>* s)
{
    for (u32 i = 0; i < s->aNodes.size; ++i)
        if (!s->aNodes[i].bDeleted) return i;

    return s->aNodes.size;
}

template<typename T, u32 CAP>
inline s64
PoolLastIdx(Pool<T, CAP>* s)
{
    for (s64 i = s->aNodes.size - 1; i >= 0; --i)
        if (!s->aNodes[i].bDeleted) return i;

    return s->aNodes.size;
}

template<typename T, u32 CAP>
inline s64
PoolNextIdx(Pool<T, CAP>* s, s64 i)
{
    do ++i;
    while (i < s->aNodes.size && s->aNodes[i].bDeleted);

    return i;
}

template<typename T, u32 CAP>
inline s64
PoolPrevIdx(Pool<T, CAP>* s, s64 i)
{
    do --i;
    while (i >= 0 && s->aNodes[i].bDeleted);

    return i;
}

template<typename T, u32 CAP>
inline void
PoolDestroy(Pool<T, CAP>* s)
{
    mtx_destroy(&s->mtx);
    mtx_destroy(&s->mtxRef);
}

/* does not increment the reference counter */
template<typename T, u32 CAP>
[[nodiscard]] inline PoolHnd
PoolRent(Pool<T, CAP>* s)
{
    guard::Mtx lock(&s->mtx);

    PoolHnd ret = std::numeric_limits<PoolHnd>::max();

    if (s->nOccupied >= CAP)
    {
#ifndef NDEBUG
        fputs("[MemPool]: no free element, returning NPOS", stderr);
#endif
        return ret;
    }

    ++s->nOccupied;

    if (s->aFreeIdxs.size > 0) ret = *ArrPop(&s->aFreeIdxs);
    else ret = ArrFakePush(&s->aNodes);

    s->aNodes[ret].bDeleted = false;
    return ret;
}

/* return handle ignoring the reference counter */
template<typename T, u32 CAP>
inline void
PoolReturn(Pool<T, CAP>* s, PoolHnd hnd)
{
    guard::Mtx lock(&s->mtx);

    --s->nOccupied;
    assert(s->nOccupied < CAP && "[Pool]: nothing to return");

    if (hnd == ArrSize(&s->aNodes) - 1) ArrFakePop(&s->aNodes);
    else
    {
        ArrPush(&s->aFreeIdxs, hnd);
        auto& node = s->aNodes[hnd];
        assert(!node.bDeleted && "[Pool]: returning already deleted node");
        node.nRefs = 0;
        node.bDeleted = true;
    }
}

template<typename T, u32 CAP>
inline void
PoolRef(Pool<T, CAP>* s, PoolHnd hnd)
{
    guard::Mtx lock(&s->mtxRef);

    assert(!s->aNodes[hnd].bDeleted && "[Pool]: can't ref deleted node");
    ++s->aNodes[hnd].nRefs;
}

/* calls return when reference counter hits 0 */
template<typename T, u32 CAP>
inline void
PoolUnref(Pool<T, CAP>* s, PoolHnd hnd)
{
    guard::Mtx lock(&s->mtxRef);

    auto& node = s->aNodes[hnd];

    assert(!node.bDeleted && "[Pool]: can't ref deleted node");
    assert(node.nRefs > 0 && "[Pool]: unrefing node with 0 references");
    --node.nRefs;

    if (node.nRefs < 1)
        PoolReturn(s, hnd);
}

} /* namespace adt */
