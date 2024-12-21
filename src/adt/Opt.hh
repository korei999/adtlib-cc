#pragma once

#include <cassert>
#include <new> /* IWYU pragma: keep */

namespace adt
{

template<typename T>
struct Opt
{
    T data {};
    bool bHasValue = false;

    constexpr Opt() = default;
    constexpr Opt(const T& x, bool _bHasValue = true)
    {
        bHasValue = _bHasValue;
        data = x;
    }

    constexpr T& getData() { assert(bHasValue && "[Opt]: has no data"); return data; }

    constexpr operator bool() const
    {
        return this->bHasValue;
    }
};

} /* namespace adt */
