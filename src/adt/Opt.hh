#pragma once

#include <cassert>

namespace adt
{

template<typename T>
class Opt
{
    T m_data {};
    bool m_bHasValue = false;

public:
    constexpr Opt() = default;
    constexpr Opt(const T& x, bool _bHasValue = true)
    {
        m_bHasValue = _bHasValue;
        m_data = x;
    }

    constexpr T& data() { assert(m_bHasValue && "[Opt]: has no data"); return m_data; }

    constexpr operator bool() const
    {
        return this->m_bHasValue;
    }
};

} /* namespace adt */
