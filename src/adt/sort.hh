#pragma once

#include "utils.hh"

#include <cmath>
#include <cstdio>

namespace adt
{

constexpr u32
HeapParentI(const u32 i)
{
    return ((i + 1) / 2) - 1;
}

constexpr u32
HeapLeftI(const u32 i)
{
    return ((i + 1) * 2) - 1;
}

constexpr u32
HeapRightI(const u32 i)
{
    return HeapLeftI(i) + 1;
}

constexpr void
maxHeapify(auto* a, const u32 size, u32 i)
{
    long largest, left, right;

again:
    left = HeapLeftI(i);
    right = HeapRightI(i);

    if (left < size && a[left] > a[i])
        largest = left;
    else largest = i;

    if (right < size && a[right] > a[largest])
        largest = right;

    if (largest != i)
    {
        utils::swap(&a[i], &a[largest]);
        i = largest;
        goto again;
    }
}

namespace sort
{

enum ORDER : u8 { INC, DEC };

constexpr bool
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
        for (long i = size - 2; i >= 0; --i)
            if (a[i + 1] > a[i]) return false;
    }

    return true;
}

constexpr bool
sorted(const auto& a, const ORDER eOrder = INC)
{
    return sorted(a.pData, a.size, eOrder);
}

template<typename T, auto FN_CMP = utils::compare<T>>
constexpr void
insertion(T* a, int l, int h)
{
    for (int i = l + 1; i < h + 1; i++)
    {
        T key = a[i];
        int j = i;
        for (; j > l && FN_CMP(a[j - 1], key) > 0; --j)
            a[j] = a[j - 1];

        a[j] = key;
    }
}

template<template<typename> typename CON_T, typename T, auto FN_CMP = utils::compare<T>>
constexpr void
insertion(CON_T<T>* a)
{
    insertion<T, FN_CMP>(a->pData, 0, a->size - 1);
}

constexpr void
heapMax(auto* a, const u32 size)
{
    u32 heapSize = size;
    for (int p = HeapParentI(heapSize); p >= 0; --p)
        maxHeapify(a, heapSize, p);

    for (long i = size - 1; i > 0; --i)
    {
        utils::swap(&a[i], &a[0]);

        --heapSize;
        maxHeapify(a, heapSize, 0);
    }
}

template<typename T>
[[nodiscard]]
constexpr int
partition(T a[], int l, int h)
{
    int p = h, firstHigh = l;

    for (int i = l; i < h; ++i)
    {
        if (a[i] < a[p])
        {
            utils::swap(&a[i], &a[firstHigh]);
            firstHigh++;
        }
    }

    utils::swap(&a[p], &a[firstHigh]);

    return firstHigh;
}

constexpr auto
median3(const auto& x, const auto& y, const auto& z)
{
    if ((x < y && y < z) || (z < y && y < x)) return y;
    else if ((y < x && x < z) || (z < x && x < y)) return x;
    else return z;
}

template<typename T, auto FN_CMP = utils::compare<T>>
constexpr void
quick(T* a, int l, int h)
{
    if (l < h)
    {
        if ((h - l + 1) < 64)
        {
            insertion<T, FN_CMP>(a, l, h);
            return;
        }

        int pivotIdx = median3(l, (l + h) / 2, h);
        T pivot = a[pivotIdx];
        int i = l, j = h;

        while (i <= j)
        {
            while (FN_CMP(a[i], pivot) < 0) ++i;
            while (FN_CMP(a[j], pivot) > 0) --j;

            if (i <= j)
            {
                utils::swap(&a[i], &a[j]);
                ++i, --j;
            }
        }

        if (l < j) quick(a, l, j);
        if (i < h) quick(a, i, h);
    }
}

template<template<typename> typename CON_T, typename T, auto FN_CMP = utils::compare<T>>
constexpr void
quick(CON_T<T>* pArr)
{
    quick<T, FN_CMP>(pArr->pData, 0, pArr->size - 1);
}

} /* namespace sort */
} /* namespace adt */
