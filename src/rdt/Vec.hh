#pragma once

#include "Allocator.hh"

#include <utility>
#include <new>

namespace rdt
{

template<typename T, typename ALLOC_T = StdAllocator>
struct Vec
{
protected:
    ADT_NO_UNIQUE_ADDRESS ALLOC_T m_alloc {};
    ssize m_cap {};
    ssize m_size {};
    T* m_pData {};

    /* */

public:

    Vec() = default;
    Vec(ssize prealloc);
    ~Vec();

    /* */

    T& operator[](ssize i) { return m_pData[i]; }
    const T& operator[](ssize i) const { return m_pData[i]; }

    T* data() { return m_pData; }
    const T* data() const { return m_pData; }

    ssize size() const { return m_size; }
    ssize cap() const { return m_cap; }

    ALLOC_T& allocator() { return m_alloc; }
    const ALLOC_T& allocator() const { return m_alloc; }

    ssize push(const T& x);

    template<typename ...ARGS>
    ssize emplace(ARGS&&... args);

protected:
    void grow();
};

template<typename T, typename ALLOC_T>
Vec<T, ALLOC_T>::Vec(ssize prealloc)
    : m_cap {prealloc}, m_pData {m_alloc.template mallocV<T>(prealloc)}
{
}

template<typename T, typename ALLOC_T>
Vec<T, ALLOC_T>::~Vec()
{
    if constexpr (!std::is_trivially_copyable_v<T>)
    {
        for (ssize i = 0; i < size(); ++i)
            m_pData[i].~T();
    }

    m_alloc.free(m_pData);
}

template<typename T, typename ALLOC_T>
inline ssize
Vec<T, ALLOC_T>::push(const T& x)
{
    grow();
    new(m_pData + m_size++) T {x};
    return size() - 1;
}

template<typename T, typename ALLOC_T>
inline void
Vec<T, ALLOC_T>::grow()
{
    if (size() >= cap())
    {
        const ssize newCap = size() <= 0 ? 2 : size() * 2;
        T* pNew = m_alloc.template mallocV<T>(newCap);

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            memcpy(pNew, m_pData, cap() * sizeof(T));
        }
        else
        {
            for (ssize i = 0; i < cap(); ++i)
            {
                new(pNew + i) T {std::move(m_pData[i])};
                m_pData[i].~T();
            }
        }

        m_alloc.free(m_pData);

        m_pData = pNew;
        m_cap = newCap;
    }
}

template<typename T, typename ALLOC_T>
template<typename ...ARGS>
inline ssize
Vec<T, ALLOC_T>::emplace(ARGS&&... args)
{
    grow();
    new(m_pData + m_size++) T {std::forward<ARGS>(args)...};
    return size() - 1;
}

} /* namespace rdt */
