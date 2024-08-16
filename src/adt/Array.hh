#pragma once

#include "Allocator.hh"

#include <assert.h>

namespace adt
{

template<typename T>
struct Array
{
    Allocator* pAlloc = nullptr;
    T* pData = nullptr;
    u32 size = 0;
    u32 cap = 0;

    Array() = default;
    Array(Allocator* pA, u32 capacity = SIZE_MIN);

    T& operator[](u32 i) { assert(i < size && "out of range access"); return pData[i]; }
    const T& operator[](u32 i) const { assert(i < size && "out of range access"); return pData[i]; }

    struct It
    {
        T* p;

        It(T* pFirst) : p{pFirst} {}

        T& operator*() { return *p; }
        T* operator->() { return p; }

        It operator++() { p++; return *this; }
        It operator++(int) { T* tmp = p++; return tmp; }
        It operator--() { p--; return *this; }
        It operator--(int) { char* tmp = p--; return tmp; }

        friend constexpr bool operator==(const It& l, const It& r) { return l.p == r.p; }
        friend constexpr bool operator!=(const It& l, const It& r) { return l.p != r.p; }
    };

    It begin() { return {&this->pData[0]}; }
    It end() { return {&this->pData[this->size]}; }
    It rbegin() { return {&this->pData[this->size - 1]}; }
    It rend() { return {this->pData - 1}; }
};

template<typename T>
inline T*
ArrayPush(Array<T>* s, const T& data)
{
    assert(s->cap > 0 && "uninitialized array push");

    if (s->size >= s->cap)
        ArrayGrow(s, s->cap * 2);

    s->pData[s->size++] = data;

    return &s->pData[s->size - 1];
}

template<typename T>
inline void
ArrayGrow(Array<T>* s, u32 size)
{
    s->cap = size;
    s->pData = (T*)realloc(s->pAlloc, s->pData, sizeof(T), size);
}

template<typename T>
inline T&
ArrayLast(Array<T>* s)
{
    return s->pData[s->size - 1];
}

template<typename T>
inline T&
ArrayFirst(Array<T>* s)
{
    return s->pData[0];
}


template<typename T>
inline T*
ArrayPop(Array<T>* s)
{
    assert(s->size > 0 && "popping from the empty array");
    return &s->pData[--s->size];
}

template<typename T>
inline void
ArraySetSize(Array<T>* s, u32 size)
{
    if (s->size < size)
        ArrayGrow(s, size);

    s->size = size;
}

template<typename T>
inline void
ArrayDestroy(Array<T>* s)
{
    free(s->pAlloc, s->pData);
}

template<typename T>
Array<T>::Array(Allocator* pA, u32 capacity)
    : pAlloc{pA},
      pData{(T*)alloc(pA, capacity, sizeof(T))},
      size{0},
      cap{capacity} {}

} /* namespace adt */
