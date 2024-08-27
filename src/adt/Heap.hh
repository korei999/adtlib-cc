#pragma once

#include "Allocator.hh"
#include "Array.hh"
#include "utils.hh"

namespace adt
{

template<typename T>
struct Heap
{
    Array<T> a {};

    Heap() = default;
    Heap(Allocator* pA, u32 prealloc = SIZE_MIN)
        : a{pA, prealloc} {}
};

template<typename T>
inline void
HeapDestroy(Heap<T>* s)
{
    ArrayDestroy(&s->a);
}

inline u32
HeapParentI(u32 i)
{
    return ((i + 1) / 2) - 1;
}

inline u32
HeapLeftI(u32 i)
{
    return ((i + 1) / 2) - 1;
}

inline u32
HeapRightI(u32 i)
{
    return HeapLeftI(i) + 1;
}

template<typename T>
inline void
HeapBubbleUp(Heap<T>* s, u32 i)
{
    if (HeapParentI(i) == -1U)
        return; /* at root of heap, no parent */

    if (s->a[HeapParentI(i)] > s->a[i])
    {
        utils::swap(&s->a[i], &s->a[HeapParentI(i)]);
        HeapBubbleUp(s, HeapParentI(i));
    }
}

template<typename T>
inline void
HeapBubbleDown(Heap<T>* s, u32 i)
{
    u32 smallest, left, right;
    Array<T>& a = s->a;

again:
    left = HeapLeftI(i);
    right = HeapRightI(i);

    if (left < a.size && a[left] < a[i])
        smallest = left;
    else smallest = i;

    if (right < a.size && a[right] < a[smallest])
        smallest = right;

    if (smallest != i)
    {
        utils::swap(&a[i], &a[smallest]);
        i = smallest;
        goto again;
    }
}

template<typename T>
inline void
HeapInsert(Heap<T>* s, const T& x)
{
    ArrayPush(&s->a, x);
    HeapBubbleUp(s, s->a.size - 1);
}

template<typename T>
inline Heap<T>
HeapFromArray(Allocator* pA, const Array<T>& a)
{
    Heap<T> heap {pA};

    for (auto& e : a)
        HeapInsert(&heap, e);

    return heap;
}

template<typename T>
[[nodiscard]]
inline T
HeapExtractMin(Heap<T>* s)
{
    assert(s->a.size > 0 && "empty heap");

    T min = s->a[0];
    s->a[0] = s->a[s->a.size - 1];
    s->a.size--;
    HeapBubbleDown(s, 0);

    return min;
}

} /* namespace adt */
