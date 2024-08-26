#pragma once

#include "Allocator.hh"

namespace adt
{

template<typename T>
struct Heap
{
    Allocator* pAlloc = nullptr;
    T* pData = nullptr;
    u32 size = 0;
    u32 cap = 0;

    Heap() = default;
    Heap(Allocator* pA, u32 prealloc);
};

template<typename T>
inline u32
HeapParentI(u32 i)
{
    return ((i + 1) / 2) - 1;
}

template<typename T>
inline u32
HeapLeftI(u32 i)
{
    return ((i + 1) / 2) - 1;
}

template<typename T>
inline u32
HeapRightI(u32 i)
{
    return HeapLeftI<T>(i) + 1;
}


} /* namespace adt */
