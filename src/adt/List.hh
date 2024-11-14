#pragma once

#include "Allocator.hh"
#include "print.hh"

namespace adt
{

#define ADT_LIST_FOREACH_SAFE(L, IT, TMP) for (decltype((L)->pFirst) IT = (L)->pFirst, TMP = {}; IT && ((TMP) = (IT)->pNext, true); (IT) = (TMP))

template<typename T>
struct ListNode
{
    ListNode* pPrev;
    ListNode* pNext;
    T data;
};

template<typename T>
struct ListBase
{
    ListNode<T>* pFirst {};
    ListNode<T>* pLast {};
    u32 size {};

    struct It
    {
        ListNode<T>* s = nullptr;

        It(ListNode<T>* p) : s {p} {}

        T& operator*() { return s->data; }
        T* operator->() { return &s->data; }

        It operator++() { return s = s->pNext; }
        It operator++(int) { T* tmp = s++; return tmp; }

        It operator--() { return s = s->pPrev; }
        It operator--(int) { T* tmp = s--; return tmp; }

        friend constexpr bool operator==(const It l, const It r) { return l.s == r.s; }
        friend constexpr bool operator!=(const It l, const It r) { return l.s != r.s; }
    };

    It begin() { return {this->pFirst}; }
    It end() { return nullptr; }
    It rbegin() { return {this->pLast}; }
    It rend() { return nullptr; }

    const It begin() const { return {this->pFirst}; }
    const It end() const { return nullptr; }
    const It rbegin() const { return {this->pLast}; }
    const It rend() const { return nullptr; }
};

template<typename T>
constexpr void
ListDestroy(ListBase<T>* s, Allocator* pA)
{
    ADT_LIST_FOREACH_SAFE(s, it, tmp)
        free(pA, it);

    s->pFirst = s->pLast = nullptr;
}

template<typename T>
constexpr ListNode<T>*
ListNodeAlloc(Allocator* pA, const T& x)
{
    auto* pNew = (ListNode<T>*)alloc(pA, 1, sizeof(ListNode<T>));
    pNew->data = x;

    return pNew;
}

template<typename T>
constexpr ListNode<T>*
ListPushFront(ListBase<T>* s, ListNode<T>* pNew)
{
    if (!s->pFirst)
    {
        pNew->pNext = pNew->pPrev = nullptr;
        s->pLast = s->pFirst = pNew;
    }
    else
    {
        pNew->pPrev = nullptr;
        pNew->pNext = s->pFirst;
        s->pFirst->pPrev = pNew;
        s->pFirst = pNew;
    }

    ++s->size;
    return pNew;
}

template<typename T>
constexpr ListNode<T>*
ListPushBack(ListBase<T>* s, ListNode<T>* pNew)
{
    if (!s->pFirst)
    {
        pNew->pNext = pNew->pPrev = nullptr;
        s->pLast = s->pFirst = pNew;
    }
    else
    {
        pNew->pNext = nullptr;
        pNew->pPrev = s->pLast;
        s->pLast->pNext = pNew;
        s->pLast = pNew;
    }

    ++s->size;
    return pNew;
}

template<typename T>
constexpr ListNode<T>*
ListPushFront(ListBase<T>* s, Allocator* pA, const T& x)
{
    auto* pNew = ListNodeAlloc(pA, x);
    return ListPushFront(s, pNew);
}

template<typename T>
constexpr ListNode<T>*
ListPushBack(ListBase<T>* s, Allocator* pA, const T& x)
{
    auto* pNew = ListNodeAlloc(pA, x);
    return ListPushBack(s, pNew);
}

template<typename T>
constexpr void
ListRemove(ListBase<T>* s, ListNode<T>* p)
{
    assert(p && s->size > 0);

    if (p == s->pFirst && p == s->pLast)
    {
        s->pFirst = s->pLast = nullptr;
    }
    else if (p == s->pFirst)
    {
        s->pFirst = s->pFirst->pNext;
        s->pFirst->pPrev = nullptr;
    }
    else if (p == s->pLast)
    {
        s->pLast = s->pLast->pPrev;
        s->pLast->pNext = nullptr;
    }
    else 
    {
        p->pPrev->pNext = p->pNext;
        p->pNext->pPrev = p->pPrev;
    }

    --s->size;
}

template<typename T>
constexpr void
ListInsertAfter(ListBase<T>* s, ListNode<T>* pAfter, ListNode<T>* p)
{
    p->pPrev = pAfter;
    p->pNext = pAfter->pNext;

    if (p->pNext) p->pNext->pPrev = p;

    pAfter->pNext = p;

    if (pAfter == s->pLast) s->pLast = p;

    ++s->size;
}

template<typename T>
constexpr void
ListInsertBefore(ListBase<T>* s, ListNode<T>* pBefore, ListNode<T>* p)
{
    p->pNext = pBefore;
    p->pPrev = pBefore->pPrev;

    if (p->pPrev) p->pPrev->pNext = p;

    pBefore->pPrev = p;

    if (pBefore == s->pFirst) s->pFirst = p;

    ++s->size;
}

template<typename T>
struct List
{
    ListBase<T> base {};
    Allocator* pAlloc {};

    List() = default;
    List(Allocator* pA) : pAlloc(pA) {}

    ListBase<T>::It begin() { return base.begin(); }
    ListBase<T>::It end() { return base.end(); }
    ListBase<T>::It rbegin() { return base.rbegin(); }
    ListBase<T>::It rend() { return base.rend(); }

    const ListBase<T>::It begin() const { return base.begin(); }
    const ListBase<T>::It end() const { return base.end(); }
    const ListBase<T>::It rbegin() const { return base.rbegin(); }
    const ListBase<T>::It rend() const { return base.rend(); }
};

template<typename T>
constexpr ListNode<T>*
ListPushFront(List<T>* s, const T& x)
{
    return ListPushFront(&s->base, s->pAlloc, x);
}

template<typename T>
constexpr ListNode<T>*
ListPushBack(List<T>* s, const T& x)
{
    return ListPushBack(&s->base, s->pAlloc, x);
}

template<typename T>
constexpr void
ListRemove(List<T>* s, ListNode<T>* p)
{
    ListRemove(&s->base, p);
    free(s->pAlloc, p);
}

template<typename T>
constexpr void
ListDestroy(List<T>* s)
{
    ListDestroy(&s->base, s->pAlloc);
}

namespace print
{

template<typename T>
inline u32
formatToContext(Context ctx, [[maybe_unused]] FormatArgs fmtArgs, const ListBase<T>& x)
{
    if (x.size == 0)
    {
        ctx.fmt = "{}";
        ctx.fmtIdx = 0;
        return printArgs(ctx, "(empty)");
    }

    char aBuff[1024] {};
    u32 nRead = 0;
    ADT_LIST_FOREACH_SAFE(&x, it, tmp)
    {
        const char* fmt = it == x.pLast ? "{}" : "{}, ";
        nRead += toBuffer(aBuff + nRead, utils::size(aBuff) - nRead, fmt, it->data);
    }

    return print::copyBackToBuffer(ctx, aBuff, utils::size(aBuff));
}

template<typename T>
inline u32
formatToContext(Context ctx, FormatArgs fmtArgs, const List<T>& x)
{
    return formatToContext(ctx, fmtArgs, x.base);
}

} /* namespace print */

} /* namespace adt */
