#pragma once

#include "types.hh"

#include <cassert>

namespace adt
{

template<typename T>
struct Span
{
    T* m_pData {};
    ssize m_size {};

    /* */

    constexpr Span() = default;

    constexpr Span(T* pData, ssize size) noexcept
        : m_pData(pData), m_size(size) {}

    template<ssize N>
    constexpr Span(T (&aChars)[N]) noexcept
        : m_pData(aChars), m_size(N) {}

    /* */

    constexpr T* data() noexcept { return m_pData; }
    constexpr const T* data() const noexcept { return m_pData; }

    constexpr ssize getSize() const noexcept { return m_size; }

    constexpr ssize lastI() const noexcept { assert(m_size > 0 && "[Span]: empty"); return m_size - 1; }

    constexpr ssize
    idx(const T* pItem) const noexcept
    {
        ssize i = pItem - m_pData;
        assert(i >= 0 && i < m_size && "[Span]: out of range");
        return i;
    }

    constexpr T&
    operator[](ssize i) noexcept
    {
        assert(i >= 0 && i < m_size && "[Span]: out of range");
        return m_pData[i];
    }

    constexpr const T&
    operator[](ssize i) const noexcept
    {
        assert(i >= 0 && i < m_size && "[Span]: out of range");
        return m_pData[i];
    }

    constexpr operator bool() const { return m_pData != nullptr; }

    /* */

    struct It
    {
        T* s;

        It(T* pFirst) : s{pFirst} {}

        T& operator*() noexcept { return *s; }
        T* operator->() noexcept { return s; }

        It operator++() noexcept { ++s; return *this; }
        It operator++(int) noexcept { T* tmp = s++; return tmp; }

        It operator--() noexcept { --s; return *this; }
        It operator--(int) noexcept { T* tmp = s--; return tmp; }

        friend constexpr bool operator==(const It& l, const It& r) noexcept { return l.s == r.s; }
        friend constexpr bool operator!=(const It& l, const It& r) noexcept { return l.s != r.s; }
    };

    It begin()  noexcept { return {&m_pData[0]}; }
    It end()    noexcept { return {&m_pData[m_size]}; }
    It rbegin() noexcept { return {&m_pData[m_size - 1]}; }
    It rend()   noexcept { return {m_pData - 1}; }

    const It begin() const  noexcept { return {&m_pData[0]}; }
    const It end() const    noexcept { return {&m_pData[m_size]}; }
    const It rbegin() const noexcept { return {&m_pData[m_size - 1]}; }
    const It rend() const   noexcept { return {m_pData - 1}; }
};

} /* namespace adt */
