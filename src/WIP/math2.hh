#pragma once

#include "math2-inl.hh"
#include "adt/utils.hh"

#include <cmath>

namespace adt::math2
{

template<typename T>
inline bool
eq(const T& l, const T& r)
{
    return l == r;
}

template<>
inline bool
eq(const f64& l, const f64& r)
{
    return std::abs(l - r) <= EPS64*(std::abs(l) + std::abs(r) + 1.0);
}

template<>
inline bool
eq(const f32& l, const f32& r)
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

constexpr inline auto
lerp(const auto& a, const auto& b, const auto& t)
{
    return (1.0 - t)*a + t*b;
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
inline T
V2Base<T>::length() const noexcept
{
    return std::hypot(x(), y());
}

template<typename T>
inline V2Base<T>
V2Base<T>::normalized() const noexcept
{
    const T len = length();
    return {x() / len, y() / len};
}

template<typename T>
inline V2Base<T>
V2Base<T>::clamped(const V2Base& min, const V2Base& max) const noexcept
{
    V2Base r {UNINIT};

    f32 minX = utils::min(min.x(), max.x());
    f32 minY = utils::min(min.y(), max.y());

    f32 maxX = utils::max(min.x(), max.x());
    f32 maxY = utils::max(min.y(), max.y());

    r.x() = utils::clamp(x(), minX, maxX);
    r.y() = utils::clamp(y(), minY, maxY);

    return r;
}

template<typename T>
inline T
V2Base<T>::dot(const V2Base& r) const noexcept
{
    return (x() * r.x()) + (y() * r.y());
}

template<typename T>
inline T
V2Base<T>::dist(const V2Base& r) const noexcept
{
    return std::sqrt(sq(r.x() - x()) + sq(r.y() - y()));
}

template<typename T>
inline T
V2Base<T>::cross(const V2Base& r) const noexcept
{
    return x() * r.y() - y() * r.x();
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

template<typename T>
inline T
V3Base<T>::length() const noexcept
{
    return std::sqrt(sq(x()) + sq(y()) + sq(z()));
}

template<typename T>
inline V3Base<T>
V3Base<T>::normalized(const f32 len) const noexcept
{
    return {x() / len, y() / len, z() / len};
}

template<typename T>
inline V3Base<T>
V3Base<T>::normalized() const noexcept
{
    return normalized(length());
}

template<typename T>
inline T
V3Base<T>::dot(const V3Base& r) const noexcept
{
    return (x() * r.x()) + (y() * r.y()) + (z() * r.z());
}

template<typename T>
inline T
V3Base<T>::rad(const V3Base& r) const noexcept
{
    return std::acos(dot(r) / length() * r.length());
}

template<typename T>
inline T
V3Base<T>::dist(const V3Base& r) const noexcept
{
    return std::sqrt(sq(r.x() - x()) + sq(r.y() - y()) + sq(r.z() - z()));
}

template<typename T>
inline V3Base<T>
V3Base<T>::cross(const V3Base& r) const noexcept
{
    return {
        (y() * r.z()) - (r.y() * z()),
        (z() * r.x()) - (r.z() * x()),
        (x() * r.y()) - (r.x() * y())
    };
}

template<typename T>
inline bool
V4Base<T>::operator==(const V4Base& r) const noexcept
{
    return eq(xyz(), r.xyz()) && eq(w(), r.w());
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator+(const V4Base& r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4Base(
        simd::f32x4(*this) + simd::f32x4(r)
    );

#else

    return {
        x() + r.x(),
        y() + r.y(),
        z() + r.z(),
        w() + r.w()
    };

#endif
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator-() const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4(-simd::f32x4(*this));

#else

    return {
        -x(), -y(), -z(), -w()
    };

#endif
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator-(const V4Base& r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4(simd::f32x4(*this) - simd::f32x4(r));

#else

    return {
        x() - r.x(),
        y() - r.y(),
        z() - r.z(),
        w() - r.w()
    };

#endif
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator*(f32 r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4(simd::f32x4(*this) * r);

#else

    return {
        x() * r,
        y() * r,
        z() * r,
        w() * r
    };

#endif
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator*(const V4Base<T>& r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4(simd::f32x4Load(this->e) * simd::f32x4Load(r.e));

#else

    return {
        x() * r.x(),
        y() * r.y(),
        z() * r.z(),
        w() * r.w()
    };

#endif
}

template<typename T>
inline V4Base<T>&
V4Base<T>::operator*=(const V4Base& r) noexcept
{
    return *this = *this * r;
}

template<typename T>
inline V4Base<T>
V4Base<T>::operator/(f32 r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    return V4(
        simd::f32x4(*this) / simd::f32x4{r}
    );

#else

    return {
        x() / r,
        y() / r,
        z() / r,
        w() / r
    };

#endif
}

template<typename T>
inline V4Base<T>&
V4Base<T>::operator+=(const V4Base& r) noexcept
{
    return *this = *this + r;
}

template<typename T>
inline V4Base<T>&
V4Base<T>::operator-=(const V4Base& r) noexcept
{
    return *this = *this - r;
}

template<typename T>
inline V4Base<T>&
V4Base<T>::operator*=(f32 r) noexcept
{
    return *this = *this * r;
}

template<typename T>
inline V4Base<T>&
V4Base<T>::operator/=(f32 r) noexcept
{
    return *this = *this / r;
}

template<typename T>
inline T
V4Base<T>::length() const noexcept
{
    return std::sqrt(sq(x()) + sq(y()) + sq(z()) + sq(w()));
}

template<typename T>
inline V4Base<T>
V4Base<T>::normalized() const noexcept
{
    const T len = length();
    return {x() / len, y() / len, z() / len, w() / len};
}

template<typename T>
inline T
V4Base<T>::dot(const V4Base& r) const noexcept
{
#if defined ADT_SSE4_2 && 0

    __m128 left = _mm_set_ps(l.w, l.z, l.y, l.x);
    __m128 right = _mm_set_ps(r.w, r.z, r.y, r.x);
    return _mm_cvtss_f32(_mm_dp_ps(left, right, 0xff));

#else

    return (x() * r.x()) + (y() * r.y()) + (z() * r.z()) + (w() * r.w());

#endif
}

template<typename T>
inline
M2Base<T>::M2Base(InitFlag) noexcept
    : m_a
    {
        1, 0,
        0, 1
    }
{
}

template<typename T>
inline T
M2Base<T>::det() const noexcept
{
    const M2Base& d = *this;
    return d[0]*d[3] - d[1]*d[2];
}

template<typename T>
constexpr inline
M3Base<T>::M3Base(int) noexcept
    : m_a
    {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    }
{
}

template<typename T>
inline constexpr
M3Base<T>::M3Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8) noexcept
    : m_a
    {
        _0, _1, _2,
        _3, _4, _5,
        _6, _7, _8
    }
{
}

template<typename T>
inline T
M3Base<T>::det() const noexcept
{
    const auto& e = m_a;
    return (
        e[0][0] * (e[1][1] * e[2][2] - e[2][1] * e[1][2]) -
        e[0][1] * (e[1][0] * e[2][2] - e[1][2] * e[2][0]) +
        e[0][2] * (e[1][0] * e[2][1] - e[1][1] * e[2][0])
    );
}

template<typename T>
inline M3Base<T>
M3Base<T>::minors() const noexcept
{
    const auto& e = m_a;
    return {
        M2Base<T>({e[1][1], e[1][2], e[2][1], e[2][2]}).det(),
        M2Base<T>({e[1][0], e[1][2], e[2][0], e[2][2]}).det(),
        M2Base<T>({e[1][0], e[1][1], e[2][0], e[2][1]}).det(),

        M2Base<T>({e[0][1], e[0][2], e[2][1], e[2][2]}).det(),
        M2Base<T>({e[0][0], e[0][2], e[2][0], e[2][2]}).det(),
        M2Base<T>({e[0][0], e[0][1], e[2][0], e[2][1]}).det(),

        M2Base<T>({e[0][1], e[0][2], e[1][1], e[1][2]}).det(),
        M2Base<T>({e[0][0], e[0][2], e[1][0], e[1][2]}).det(),
        M2Base<T>({e[0][0], e[0][1], e[1][0], e[1][1]}).det()
    };
}

template<typename T>
inline M3Base<T>
M3Base<T>::cofactors() const noexcept
{
    M3 m = minors();
    auto& e = m.m_a;

    e[0][0] *= +1, e[0][1] *= -1, e[0][2] *= +1;
    e[1][0] *= -1, e[1][1] *= +1, e[1][2] *= -1;
    e[2][0] *= +1, e[2][1] *= -1, e[2][2] *= +1;

    return m;
}


template<typename T>
inline M3Base<T>
M3Base<T>::transposed() const noexcept
{
    auto& e = m_a;
    return {
        e[0][0], e[1][0], e[2][0],
        e[0][1], e[1][1], e[2][1],
        e[0][2], e[1][2], e[2][2]
    };
}

template<typename T>
inline M3Base<T>
M3Base<T>::adj() const noexcept
{
    return cofactors().transposed();
}

template<typename T>
inline M3Base<T>
M3Base<T>::inv() const noexcept
{
    return (1.0f/det()) * adj();
}

template<typename T>
inline M3Base<T>
M3Base<T>::normal() const noexcept
{
    return inv().transposed();
}

template<typename T>
inline M3Base<T>
M3Base<T>::scaled(const f32 s) const noexcept
{
    M3 sm {
        s, 0, 0,
        0, s, 0,
        0, 0, 1
    };

    return *this * sm;
}

template<typename T>
inline M3Base<T>
M3Base<T>::scaled(const V2Base<T> s) const noexcept
{
    M3 sm {
        s.x, 0,   0,
        0,   s.y, 0,
        0,   0,   1
    };

    return *this * sm;
}

template<typename T>
inline bool
M3Base<T>::operator==(const M3Base& r) const noexcept
{
    for (int i = 0; i < 9; ++i)
        if (!eq((*this)[i], r[i]))
            return false;

    return true;
}

template<typename T>
inline M3Base<T>
M3Base<T>::operator*(const f32 r) const noexcept
{
    M3 m {UNINIT};

    for (int i = 0; i < 9; ++i)
        m[i] = (*this)[i] * r;

    return m;
}

template<typename T>
inline M3Base<T>&
M3Base<T>::operator*=(const f32 r) noexcept
{
    for (int i = 0; i < 9; ++i)
        (*this)[i] *= r;

    return *this;
}

template<typename T>
inline V3Base<T>
M3Base<T>::operator*(const V3Base<T>& r) const noexcept
{
    return v3s()[0] * r.x() + v3s()[1] * r.y() + v3s()[2] * r.z();
}

template<typename T>
inline M3Base<T>
M3Base<T>::operator*(const M3Base& r) const noexcept
{
    M3 m {UNINIT};

    m.v3s()[0] = *this * r.v3s()[0];
    m.v3s()[1] = *this * r.v3s()[1];
    m.v3s()[2] = *this * r.v3s()[2];

    return m;
}

template<typename T>
inline M3Base<T>&
M3Base<T>::operator*=(const M3Base& r) noexcept
{
    return *this = *this * r;
}

template<typename T>
inline constexpr
M4Base<T>::M4Base(int) noexcept
    : m_a
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }
{
}

template<typename T>
inline constexpr
M4Base<T>::M4Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8, T _9, T _10, T _11, T _12, T _13, T _14, T _15) noexcept
    : m_a
    {
        _0, _1, _2, _3,
        _4, _5, _6, _7,
        _8, _9, _10, _11,
        _12, _13, _14, _15
    }
{
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

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math2::M2Base<T>& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\t(", x[0], ", ", x[1],
        "\n\t ", x[2], ", ", x[3], ")"
    );
}

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math2::M3Base<T>& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\t(", x[0], ", ", x[1], ", ", x[2],
        "\n\t ", x[3], ", ", x[4], ", ", x[5],
        "\n\t ", x[6], ", ", x[7], ", ", x[8], ")"
    );
}

} /* namespace adt::print */
