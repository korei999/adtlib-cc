#pragma once

#include "utils.hh"
#include "Span.hh"

namespace adt
{

inline constexpr u32
HeapParentI(const u32 i)
{
    return ((i + 1) / 2) - 1;
}

inline constexpr u32
HeapLeftI(const u32 i)
{
    return ((i + 1) * 2) - 1;
}

inline constexpr u32
HeapRightI(const u32 i)
{
    return HeapLeftI(i) + 1;
}

inline constexpr void
maxHeapify(auto* a, const u32 size, u32 i)
{
    s64 largest, left, right;

again:
    left = HeapLeftI(i);
    right = HeapRightI(i);

    if (left < size && a[left] > a[i])
        largest = left;
    else largest = i;

    if (right < size && a[right] > a[largest])
        largest = right;

    if (largest != (s64)i)
    {
        utils::swap(&a[i], &a[largest]);
        i = largest;
        goto again;
    }
}

namespace sort
{

enum ORDER : u8 { INC, DEC };

inline constexpr bool
sorted(const auto* a, const u32 size, const ORDER eOrder = INC)
{
    if (size <= 1) return true;

    if (eOrder == ORDER::INC)
    {
        for (u32 i = 1; i < size; ++i)
            if (a[i - 1] > a[i]) return false;
    }
    else
    {
        for (s64 i = size - 2; i >= 0; --i)
            if (a[i + 1] > a[i]) return false;
    }

    return true;
}

inline constexpr bool
sorted(const auto& a, const ORDER eOrder = INC)
{
    return sorted(a.data(), a.getSize(), eOrder);
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
insertion(T* a, s64 l, s64 h)
{
    for (s64 i = l + 1; i < h + 1; ++i)
    {
        T key = a[i];
        s64 j = i;
        for (; j > l && FN_CMP(a[j - 1], key) > 0; --j)
            a[j] = a[j - 1];

        a[j] = key;
    }
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
insertion(Span<T> s)
{
    insertion<T, FN_CMP>(s.data(), 0, s.lastI());
}

template<template<typename> typename CON_T, typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
insertion(CON_T<T>* a)
{
    if (a->getSize() <= 1) return;

    insertion<T, FN_CMP>(a->data(), 0, a->getSize() - 1);
}

inline constexpr void
heapMax(auto* a, const u32 size)
{
    u32 heapSize = size;
    for (s64 p = HeapParentI(heapSize); p >= 0; --p)
        maxHeapify(a, heapSize, p);

    for (s64 i = size - 1; i > 0; --i)
    {
        utils::swap(&a[i], &a[0]);

        --heapSize;
        maxHeapify(a, heapSize, 0);
    }
}

inline constexpr auto
median3(const auto& x, const auto& y, const auto& z)
{
    if ((x < y && y < z) || (z < y && y < x)) return y;
    else if ((y < x && x < z) || (z < x && x < y)) return x;
    else return z;
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr s64
partition(T a[], s64 l, s64 r, const T& pivot)
{
    while (l <= r)
    {
        while (FN_CMP(a[l], pivot) < 0) ++l;
        while (FN_CMP(a[r], pivot) > 0) --r;

        if (l <= r) utils::swap(&a[l++], &a[r--]);
    }

    return r;
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
partition(Span<T> s, const T& pivot)
{
    partition<T, FN_CMP>(s.data(), 0, s.lastI(), pivot);
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
quick(T a[], s64 l, s64 r)
{
    if (l < r)
    {
        if ((r - l + 1) < 64)
        {
            insertion<T, FN_CMP>(a, l, r);
            return;
        }

        T pivot = a[ median3(l, (l + r) / 2, r) ];
        s64 i = l, j = r;

        while (i <= j)
        {
            while (FN_CMP(a[i], pivot) < 0) ++i;
            while (FN_CMP(a[j], pivot) > 0) --j;

            if (i <= j) utils::swap(&a[i++], &a[j--]);
        }

        if (l < j) quick<T, FN_CMP>(a, l, j);
        if (i < r) quick<T, FN_CMP>(a, i, r);
    }
}

template<typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
quick(Span<T> s)
{
    quick<T, FN_CMP>(s.data(), 0, s.lastI());
}

template<template<typename> typename CON_T, typename T, auto FN_CMP = utils::compare<T>>
inline constexpr void
quick(CON_T<T>* pArrayContainer)
{
    if (pArrayContainer->getSize() <= 1) return;
    quick<T, FN_CMP>(pArrayContainer->data(), 0, pArrayContainer->getSize() - 1);
}

} /* namespace sort */
} /* namespace adt */
