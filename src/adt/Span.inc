#pragma once

#include "types.hh"

namespace adt
{

template<typename T>
struct Span
{
    T* m_pData {};
    isize m_size {};

    /* */

    constexpr Span() = default;

    constexpr Span(const T* pData, isize size) noexcept
        : m_pData(const_cast<T*>(pData)), m_size(size) {}

    template<isize N>
    constexpr Span(T (&aChars)[N]) noexcept
        : m_pData(aChars), m_size(N) {}

    /* */

    constexpr bool empty() const { return m_size <= 0; }

    constexpr T* data() noexcept { return m_pData; }
    constexpr const T* data() const noexcept { return m_pData; }

    constexpr isize size() const noexcept { return m_size; }

    constexpr isize lastI() const noexcept;

    constexpr T& first() noexcept { return operator[](0); }
    constexpr const T& first() const noexcept { return operator[](0); }

    constexpr T& last() noexcept { return operator[](m_size - 1); }
    constexpr const T& last() const noexcept { return operator[](m_size - 1); }

    constexpr isize idx(const T* pItem) const noexcept;

    constexpr T& operator[](isize i) noexcept;

    constexpr const T& operator[](isize i) const noexcept;

    constexpr operator bool() const { return m_pData != nullptr; }

    /* */

    T* begin()  noexcept { return {&m_pData[0]}; }
    T* end()    noexcept { return {&m_pData[m_size]}; }
    T* rbegin() noexcept { return {&m_pData[m_size - 1]}; }
    T* rend()   noexcept { return {m_pData - 1}; }

    const T* begin() const  noexcept { return {&m_pData[0]}; }
    const T* end() const    noexcept { return {&m_pData[m_size]}; }
    const T* rbegin() const noexcept { return {&m_pData[m_size - 1]}; }
    const T* rend() const   noexcept { return {m_pData - 1}; }
};

} /* namespace adt */
