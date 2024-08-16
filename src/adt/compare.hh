#pragma once

#include "types.hh"

namespace adt
{

/* negative is l < r, positive if l > r, 0 if l == r */
template<typename T>
constexpr s64
compare(const T& l, const T& r)
{
    return l - r;
}

} /* namespace adt */
