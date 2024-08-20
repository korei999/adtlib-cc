#pragma once

#include "Allocator.hh"

#include <assert.h>

namespace adt
{

template<typename T> struct Queue;

template<typename T> bool QueueEmpty(Queue<T>* s);
template<typename T> inline T* QueuePushBack(Queue<T>* s, const T& val);
template<typename T> inline void QueueResize(Queue<T>* s, u32 size);
template<typename T> inline T* QueuePopFront(Queue<T>* s);
template<typename T> inline void QueueDestroy(Queue<T>*s);

template<typename T> inline int QueueNextI(Queue<T>*s, int i) { return (i + 1) >= s->capacity ? 0 : (i + 1); }
template<typename T> inline int QueuePrevI(Queue<T>* s, int i) { return (i - 1) < 0 ? s->capacity - 1 : (i - 1); }
template<typename T> inline int QueueFirstI(Queue<T>* s) { return QueueEmpty(s) ? -1 : s->first; }
template<typename T> inline int QueueLastI(Queue<T>* s) { return QueueEmpty(s) ? 0 : s->last - 1; }

template<typename T>
struct Queue
{
    Allocator* pAlloc {};
    T* pData {};
    int size {};
    int capacity {};
    int first {};
    int last {};

    Queue() = default;
    Queue(Allocator* p) : Queue(p, SIZE_MIN) {}
    Queue(Allocator* p, u32 prealloc)
        : pAlloc(p),
          pData{(T*)alloc(pAlloc, prealloc, sizeof(T))},
          capacity(prealloc) {}

    T& operator[](int i) { return pData[i]; }
    const T& operator[](int i) const { return pData[i]; }
};

template<typename T>
bool
QueueEmpty(Queue<T>* s)
{
    return s->size == 0;
}

template<typename T>
inline T*
QueuePushBack(Queue<T>* s, const T& val)
{
    if (s->size >= s->capacity)
        QueueResize(s, s->capacity * 2);
    
    int i = s->last;
    int ni = QueueNextI(s, i);
    s->pData[i] = val;
    s->last = ni;
    s->size++;

    return &s->pData[i];
}

template<typename T>
inline void
QueueResize(Queue<T>* s, u32 size)
{
    auto nQ = Queue<T>(s->pAlloc, size);

    for (int i = QueueFirstI(s), t = 0; t < s->size; i = QueueNextI(s, i), t++)
        QueuePushBack(&nQ, s->pData[i]);

    QueueDestroy(s);
    *s = nQ;
}


template<typename T>
inline T*
QueuePopFront(Queue<T>* s)
{
    assert(s->size > 0);

    T* ret = &s->pData[s->first];
    s->first = QueueNextI(s, s->first);
    s->size--;

    return ret;
}

template<typename T>
inline void
QueueDestroy(Queue<T>*s)
{
    free(s->pAlloc, s->pData);
}

} /* namespace adt */
