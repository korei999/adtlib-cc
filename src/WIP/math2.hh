#pragma once

#include "math2-inl.hh"

#include <cmath>

namespace adt::math2
{

inline bool
eq(const f64 l, const f64 r)
{
    return std::abs(l - r) <= EPS64*(std::abs(l) + std::abs(r) + 1.0);
}

inline bool
eq(const f32 l, const f32 r)
{
    return std::abs(l - r) <= EPS32*(std::abs(l) + std::abs(r) + 1.0f);
}

constexpr inline auto sq(const auto& x) { return x * x; }
constexpr inline auto cube(const auto& x) { return x*x*x; }

constexpr inline i64
sign(i64 x)
{
    return (x > 0) - (x < 0);
}

template<typename T>
inline bool
V2Base<T>::operator==(const V2Base& r) const noexcept
{
    return eq(x(), r.x()) && eq(y(), r.y());
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-() noexcept
{
    return {-x(), -y()};
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator+(const V2Base& r) const noexcept
{
    return {
        x() + r.x(),
        y() + r.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-(const V2Base& r) const noexcept
{
    return {
        x() - r.x(),
        y() - r.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator*(f32 s) const noexcept
{
    return {
        x() * s,
        y() * s
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator*(const V2Base& r) const noexcept
{
    return {
        x() * r.x(),
        y() * r.y()
    };
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator*=(const V2Base& r) noexcept
{
    return *this = *this * r;
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator/(f32 s) const noexcept
{
    return {
        x() / s,
        y() / s
    };
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator+=(const V2Base& r) noexcept
{
    return *this = *this + r;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator-=(const V2Base& r) noexcept
{
    return *this = *this - r;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator*=(f32 r) noexcept
{
    return *this = *this * r;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator/=(f32 r) noexcept
{
    return *this = *this / r;
}

template<typename T>
inline bool
V3Base<T>::operator==(const V3Base& r) const noexcept
{
    return eq(xy(), r.xy()) && eq(z(), r.z());
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator+(const V3Base& r) const noexcept
{
    return {
        x() + r.x(),
        y() + r.y(),
        z() + r.z()
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator-(const V3Base& r) const noexcept
{
    return {
        x() - r.x(),
        y() - r.y(),
        z() - r.z()
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator-() const noexcept
{
    return {
        -x(), -y(), -z()
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator*(f32 s) const noexcept
{
    return {
        x() * s,
        y() * s,
        z() * s
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator*(const V3Base& r) const noexcept
{
    return {
        x() * r.x(),
        y() * r.y(),
        z() * r.z()
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator/(f32 s) const noexcept
{
    return {
        x() / s,
        y() / s,
        z() / s
    };
}

template<typename T>
inline V3Base<T>
V3Base<T>::operator+(f32 b) const noexcept
{
    return {
        x() + b,
        y() + b,
        z() + b,
    };
}

template<typename T>
inline V3Base<T>&
V3Base<T>::operator+=(f32 b) noexcept
{
    return *this = *this + b;
}

template<typename T>
inline V3Base<T>&
V3Base<T>::operator+=(const V3Base& r) noexcept
{
    return *this = *this + r;
}

template<typename T>
inline V3Base<T>&
V3Base<T>::operator-=(const V3Base<T>& r) noexcept
{
    return *this = *this - r;
}

template<typename T>
inline V3Base<T>&
V3Base<T>::operator*=(f32 s) noexcept
{
    return *this = *this * s;
}

template<typename T>
inline V3Base<T>&
V3Base<T>::operator/=(f32 s) noexcept
{
    return *this = *this / s;
}

} /* namespace adt::math2 */

namespace adt::print
{

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math2::V2Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y());
}

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math2::V3Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y(), x.z());
}

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math2::V4Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y(), x.z(), x.w());
}

} /* namespace adt::print */
