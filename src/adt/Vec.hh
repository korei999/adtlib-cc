#pragma once

#include "Allocator.hh"
#include "utils.hh"
#include "print.hh"

#include <cassert>

namespace adt
{

#define ADT_VEC_FOREACH_I(A, I) for (u32 I = 0; I < (A)->size; I++)
#define ADT_VEC_FOREACH_I_REV(A, I) for (u32 I = (A)->size - 1; I != -1U ; I--)

/* Dynamic array (aka Vector), use outside Allocator for each allocating operation explicitly */
template<typename T>
struct VecBase
{
    T* pData = nullptr;
    u32 size = 0;
    u32 capacity = 0;

    VecBase() = default;
    VecBase(Allocator* p, u32 prealloc = 1)
        : pData((T*)p->alloc(prealloc, sizeof(T))),
          size(0),
          capacity(prealloc) {}

    T& operator[](u32 i)             { assert(i < size && "[Vec] out of size"); return pData[i]; }
    const T& operator[](u32 i) const { assert(i < size && "[Vec] out of size"); return pData[i]; }

    void _grow(Allocator* p, u32 newCapacity);
    u32 push(Allocator* p, const T& data);
    [[nodiscard]] T& last();
    [[nodiscard]] const T& last() const;
    [[nodiscard]] T& first();
    [[nodiscard]] const T& first() const;
    T* pop();
    void setSize(Allocator* p, u32 size);
    void setCap(Allocator* p, u32 cap);
    void swapWithLast(u32 i);
    void popAsLast(u32 i);
    [[nodiscard]] u32 idx(const T* x);
    [[nodiscard]] u32 lastI();
    void destroy(Allocator* p);
    [[nodiscard]] u32 getSize() const;
    u32 getCap() const;
    [[nodiscard]] T* data();
    void zeroOut();
    [[nodiscard]] VecBase<T> clone(Allocator* pAlloc);

    struct It
    {
        T* s;

        It(T* pFirst) : s{pFirst} {}

        T& operator*() { return *s; }
        T* operator->() { return s; }

        It operator++() { ++s; return *this; }
        It operator++(int) { T* tmp = s++; return tmp; }

        It operator--() { --s; return *this; }
        It operator--(int) { T* tmp = s--; return tmp; }

        friend constexpr bool operator==(const It& l, const It& r) { return l.s == r.s; }
        friend constexpr bool operator!=(const It& l, const It& r) { return l.s != r.s; }
    };

    It begin() { return {&this->pData[0]}; }
    It end() { return {&this->pData[this->size]}; }
    It rbegin() { return {&this->pData[this->size - 1]}; }
    It rend() { return {this->pData - 1}; }

    const It begin() const { return {&this->pData[0]}; }
    const It end() const { return {&this->pData[this->size]}; }
    const It rbegin() const { return {&this->pData[this->size - 1]}; }
    const It rend() const { return {this->pData - 1}; }
};

template<typename T>
inline void
VecBase<T>::_grow(Allocator* p, u32 newCapacity)
{
    assert(newCapacity * sizeof(T) > 0);
    this->capacity = newCapacity;
    this->pData = (T*)p->realloc(this->pData, newCapacity, sizeof(T));
}

template<typename T>
inline u32
VecBase<T>::push(Allocator* p, const T& data)
{
    if (this->size >= this->capacity) this->_grow(p, utils::max(this->capacity * 2U, u32(SIZE_MIN)));

    this->pData[this->size++] = data;
    return this->size - 1;
}

template<typename T>
inline T&
VecBase<T>::last()
{
    assert(this->size > 0 && "[Vec]: empty");
    return this->pData[this->size - 1];
}

template<typename T>
inline const T&
VecBase<T>::last() const
{
    assert(this->size > 0 && "[Vec]: empty");
    return this->pData[this->size - 1];
}

template<typename T>
inline T&
VecBase<T>::first()
{
    return this->pData[0];
}

template<typename T>
inline const T&
VecBase<T>::first() const
{
    return this->pData[0];
}

template<typename T>
inline T*
VecBase<T>::pop()
{
    assert(this->size > 0 && "[Vec]: pop from empty");
    return &this->pData[--this->size];
}

template<typename T>
inline void
VecBase<T>::setSize(Allocator* p, u32 size)
{
    if (this->capacity < size) this->_grow(p, size);

    this->size = size;
}

template<typename T>
inline void
VecBase<T>::setCap(Allocator* p, u32 cap)
{
    this->pData = (T*)p->realloc(this->pData, cap, sizeof(T));
    this->capacity = cap;

    if (this->size > cap) this->size = cap;
}

template<typename T>
inline void
VecBase<T>::swapWithLast(u32 i)
{
    utils::swap(&this->pData[i], &this->pData[this->size - 1]);
}

template<typename T>
inline void
VecBase<T>::popAsLast(u32 i)
{
    this->pData[i] = this->pData[--this->size];
}

template<typename T>
inline u32
VecBase<T>::idx(const T* x)
{
    u32 r = u32(x - this->pData);
    assert(r < this->capacity);
    return r;
}

template<typename T>
inline u32
VecBase<T>::lastI()
{
    return this->idx(&this->last());
}

template<typename T>
inline void
VecBase<T>::destroy(Allocator* p)
{
    p->free(this->pData);
}

template<typename T>
inline u32
VecBase<T>::getSize() const
{
    return this->size;
}

template<typename T>
inline u32
VecBase<T>::getCap() const
{
    return this->capacity;
}

template<typename T>
inline T*
VecBase<T>::data()
{
    return this->pData;
}

template<typename T>
inline void
VecBase<T>::zeroOut()
{
    memset(this->pData, 0, this->size * sizeof(T));
}

template<typename T>
inline VecBase<T>
VecBase<T>::clone(Allocator* pAlloc)
{
    auto nVec = VecBase<T>(pAlloc, this->capacity);
    memcpy(nVec.pData, this->pData, this->size * sizeof(T));
    nVec.size = this->size;

    return nVec;
}

/* Dynamic array (aka Vector), with Allocator* stored */
template<typename T>
struct Vec
{
    VecBase<T> base {};
    Allocator* pAlloc = nullptr;

    Vec() = default;
    Vec(Allocator* p, u32 _cap = 1) : base(p, _cap), pAlloc(p) {}

    T& operator[](u32 i) { return base[i]; }
    const T& operator[](u32 i) const { return base[i]; }

    void _grow(u32 newCapacity) { base._grow(pAlloc, newCapacity); }
    u32 push(const T& data) { return base.push(pAlloc, data); }
    [[nodiscard]] T& last() { return base.last(); }
    [[nodiscard]] const T& last() const { return base.last(); }
    [[nodiscard]] T& first() { return base.first(); }
    [[nodiscard]] const T& first() const { return base.first(); }
    T* pop() { return base.pop(); }
    void setSize(u32 size) { base.setSize(pAlloc, size); }
    void setCap(u32 cap) { base.setCap(pAlloc, cap); }
    void swapWithLast(u32 i) { base.swapWithLast(i); }
    void popAsLast(u32 i) { base.popAsLast(i); }
    [[nodiscard]] u32 idx(const T* x) { return base.idx(x); }
    [[nodiscard]] u32 lastI() { return base.lastI(); }
    void destroy() { base.destroy(pAlloc); }
    [[nodiscard]] u32 getSize() const { return base.getSize(); }
    u32 getCap() const { return base.getCap(); }
    [[nodiscard]] T* data() { return base.data(); }
    void zeroOut() { base.zeroOut(); }
    [[nodiscard]] VecBase<T> clone(Allocator* _pAlloc) { return base.clone(_pAlloc); }

    VecBase<T>::It begin() { return base.begin(); }
    VecBase<T>::It end() { return base.end(); }
    VecBase<T>::It rbegin() { return base.rbegin(); }
    VecBase<T>::It rend() { return rend(); }

    const VecBase<T>::It begin() const { return base.begin(); }
    const VecBase<T>::It end() const { return base.end(); }
    const VecBase<T>::It rbegin() const { return base.rbegin(); }
    const VecBase<T>::It rend() const { return rend(); }
};

namespace utils
{

template<typename T> [[nodiscard]] inline bool empty(const VecBase<T>* s) { return s->size == 0; }
template<typename T> [[nodiscard]] inline bool empty(const Vec<T>* s) { return empty(&s->base); }

} /* namespace utils */

namespace print
{

template<typename T>
inline u32
formatToContext(Context ctx, [[maybe_unused]] FormatArgs fmtArgs, const VecBase<T>& x)
{
    if (utils::empty(&x))
    {
        ctx.fmt = "{}";
        ctx.fmtIdx = 0;
        return printArgs(ctx, "(empty)");
    }

    char aBuff[1024] {};
    u32 nRead = 0;
    for (u32 i = 0; i < x.size; ++i)
    {
        const char* fmt;
        if constexpr (std::is_floating_point_v<T>) fmt = i == x.size - 1 ? "{:.3}" : "{:.3}, ";
        else fmt = i == x.size - 1 ? "{}" : "{}, ";

        nRead += toBuffer(aBuff + nRead, utils::size(aBuff) - nRead, fmt, x[i]);
    }

    return print::copyBackToBuffer(ctx, aBuff, utils::size(aBuff));
}

template<typename T>
inline u32
formatToContext(Context ctx, FormatArgs fmtArgs, const Vec<T>& x)
{
    return formatToContext(ctx, fmtArgs, x.base);
}

} /* namespace print */

} /* namespace adt */
