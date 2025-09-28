#pragma once

#include "math-inl.hh"
#include "adt/utils.hh"

#include <cmath>

namespace adt::math
{

constexpr inline f64 toDeg(f64 x) { return x * 180.0 / PI64; }
constexpr inline f64 toRad(f64 x) { return x * PI64 / 180.0; }
constexpr inline f32 toDeg(f32 x) { return x * 180.0f / PI32; }
constexpr inline f32 toRad(f32 x) { return x * PI32 / 180.0f; }

constexpr inline f64 toRad(i64 x) { return toRad(static_cast<f64>(x)); }
constexpr inline f64 toDeg(i64 x) { return toDeg(static_cast<f64>(x)); }
constexpr inline f32 toRad(i32 x) { return toRad(static_cast<f32>(x)); }
constexpr inline f32 toDeg(i32 x) { return toDeg(static_cast<f32>(x)); }

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

template<typename T, std::floating_point F>
constexpr T
bezier(
    const T& p0,
    const T& p1,
    const T& p2,
    const F t)
{
    return sq(1-t)*p0 + 2*(1-t)*t*p1 + sq(t)*p2;
}

template<typename T, std::floating_point F>
constexpr T
bezier(
    const T& p0,
    const T& p1,
    const T& p2,
    const T& p3,
    const F t)
{
    return cube(1-t)*p0 + 3*sq(1-t)*t*p1 + 3*(1-t)*sq(t)*p2 + cube(t)*p3;
}

template<typename T>
inline bool
V2Base<T>::operator==(const V2Base& r) const noexcept
{
    return eq(x(), r.x()) && eq(y(), r.y());
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-() const noexcept
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

inline
M2::M2(int) noexcept
    : m_a
    {
        1, 0,
        0, 1
    }
{
}

inline f32
M2::det() const noexcept
{
    auto& d = data();
    return d[0]*d[3] - d[1]*d[2];
}

constexpr inline
M3::M3(int) noexcept
    : m_a
    {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    }
{
}

inline constexpr
M3::M3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8) noexcept
    : m_a
    {
        _0, _1, _2,
        _3, _4, _5,
        _6, _7, _8
    }
{
}

inline constexpr
M3::M3(const V3& _0, const V3& _1, const V3& _2) noexcept
    : m_a
    {
        _0.x(), _0.y(), _0.z(),
        _1.x(), _1.y(), _1.z(),
        _2.x(), _2.y(), _2.z()
    }
{
}

inline f32
M3::det() const noexcept
{
    const auto& e = m_a;
    return (
        e[0][0] * (e[1][1] * e[2][2] - e[2][1] * e[1][2]) -
        e[0][1] * (e[1][0] * e[2][2] - e[1][2] * e[2][0]) +
        e[0][2] * (e[1][0] * e[2][1] - e[1][1] * e[2][0])
    );
}

inline M3
M3::minors() const noexcept
{
    const auto& e = m_a;
    return {
        M2({e[1][1], e[1][2], e[2][1], e[2][2]}).det(),
        M2({e[1][0], e[1][2], e[2][0], e[2][2]}).det(),
        M2({e[1][0], e[1][1], e[2][0], e[2][1]}).det(),

        M2({e[0][1], e[0][2], e[2][1], e[2][2]}).det(),
        M2({e[0][0], e[0][2], e[2][0], e[2][2]}).det(),
        M2({e[0][0], e[0][1], e[2][0], e[2][1]}).det(),

        M2({e[0][1], e[0][2], e[1][1], e[1][2]}).det(),
        M2({e[0][0], e[0][2], e[1][0], e[1][2]}).det(),
        M2({e[0][0], e[0][1], e[1][0], e[1][1]}).det()
    };
}

inline M3
M3::cofactors() const noexcept
{
    M3 m = minors();
    auto& e = m.m_a;

    e[0][0] *= +1, e[0][1] *= -1, e[0][2] *= +1;
    e[1][0] *= -1, e[1][1] *= +1, e[1][2] *= -1;
    e[2][0] *= +1, e[2][1] *= -1, e[2][2] *= +1;

    return m;
}


inline M3
M3::transposed() const noexcept
{
    auto& e = m_a;
    return {
        e[0][0], e[1][0], e[2][0],
        e[0][1], e[1][1], e[2][1],
        e[0][2], e[1][2], e[2][2]
    };
}

inline M3
M3::adj() const noexcept
{
    return cofactors().transposed();
}

inline M3
M3::inv() const noexcept
{
    return (1.0 / det()) * adj();
}

inline M3
M3::normal() const noexcept
{
    return inv().transposed();
}

inline M3
M3::scaled(const f32 s) const noexcept
{
    M3 sm {
        s, 0, 0,
        0, s, 0,
        0, 0, 1
    };

    return *this * sm;
}

inline M3
M3::scaled(const V2 s) const noexcept
{
    M3 sm {
        s.x(), 0,     0,
        0,     s.y(), 0,
        0,     0,     1
    };

    return *this * sm;
}

inline bool
M3::operator==(const M3& r) const noexcept
{
    for (int i = 0; i < 9; ++i)
        if (!eq(data()[i], r.data()[i]))
            return false;

    return true;
}

inline M3
M3::operator*(const f32 r) const noexcept
{
    M3 m {UNINIT};

    for (int i = 0; i < 9; ++i)
        m.data()[i] = data()[i] * r;

    return m;
}

inline M3&
M3::operator*=(const f32 r) noexcept
{
    for (int i = 0; i < 9; ++i)
        data()[i] *= r;

    return *this;
}

inline V3
M3::operator*(const V3& r) const noexcept
{
    const V3* v = &operator[](0);
    return v[0] * r.x() + v[1] * r.y() + v[2] * r.z();
}

inline M3
M3::operator*(const M3& r) const noexcept
{
    return {
        *this * r[0],
        *this * r[1],
        *this * r[2]
    };
}

inline M3&
M3::operator*=(const M3& r) noexcept
{
    return *this = *this * r;
}

inline constexpr
M4::M4(int) noexcept
    : m_a
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }
{
}

inline constexpr
M4::M4(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8, f32 _9, f32 _10, f32 _11, f32 _12, f32 _13, f32 _14, f32 _15) noexcept
    : m_a
    {
        _0, _1, _2, _3,
        _4, _5, _6, _7,
        _8, _9, _10, _11,
        _12, _13, _14, _15
    }
{
}

constexpr inline
M4::M4(const V4& _0, const V4& _1, const V4& _2, const V4& _3) noexcept
    : m_a
    {
        _0.x(), _0.y(), _0.z(), _0.w(),
        _1.x(), _1.y(), _1.z(), _1.w(),
        _2.x(), _2.y(), _2.z(), _2.w(),
        _3.x(), _3.y(), _3.z(), _3.w()
    }
{
}

inline bool
M4::operator==(const M4& r) const noexcept
{
    for (int i = 0; i < 16; ++i)
        if (!eq(data()[i], r.data()[i]))
            return false;

    return true;
}

inline M4
M4::operator*(const f32 r) const noexcept
{
    M4 m {UNINIT};

    for (int i = 0; i < 16; ++i)
        m.data()[i] = data()[i] * r;

    return m;
}

inline M4&
M4::operator*=(const f32 r) noexcept
{
    for (int i = 0; i < 16; ++i)
        data()[i] *= r;

    return *this;
}

inline V4
M4::operator*(const V4& r) const noexcept
{
#if defined ADT_AVX2 && 0

    auto x3 = v[3] * r.w;
    auto x2 = simd::fma(v[2], r.z, x3);
    auto x1 = simd::fma(v[1], r.y, x2);
    auto x0 = simd::fma(v[0], r.x, x1);

    return V4(x0);

#else

    const V4* v = &this->operator[](0);
    return v[0] * r.x() + v[1] * r.y() + v[2] * r.z() + v[3] * r.w();

#endif
}

inline M4
M4::operator*(const M4& r) const noexcept
{
    return {
        *this * r[0],
        *this * r[1],
        *this * r[2],
        *this * r[3]
    };
}

inline M4&
M4::operator*=(const M4& r) noexcept
{
    return *this = *this * r;
}

inline f32
M4::det() const noexcept
{
    auto& e = m_a;
    return (
        e[0][3] * e[1][2] * e[2][1] * e[3][0] - e[0][2] * e[1][3] * e[2][1] * e[3][0] -
        e[0][3] * e[1][1] * e[2][2] * e[3][0] + e[0][1] * e[1][3] * e[2][2] * e[3][0] +
        e[0][2] * e[1][1] * e[2][3] * e[3][0] - e[0][1] * e[1][2] * e[2][3] * e[3][0] -
        e[0][3] * e[1][2] * e[2][0] * e[3][1] + e[0][2] * e[1][3] * e[2][0] * e[3][1] +
        e[0][3] * e[1][0] * e[2][2] * e[3][1] - e[0][0] * e[1][3] * e[2][2] * e[3][1] -
        e[0][2] * e[1][0] * e[2][3] * e[3][1] + e[0][0] * e[1][2] * e[2][3] * e[3][1] +
        e[0][3] * e[1][1] * e[2][0] * e[3][2] - e[0][1] * e[1][3] * e[2][0] * e[3][2] -
        e[0][3] * e[1][0] * e[2][1] * e[3][2] + e[0][0] * e[1][3] * e[2][1] * e[3][2] +
        e[0][1] * e[1][0] * e[2][3] * e[3][2] - e[0][0] * e[1][1] * e[2][3] * e[3][2] -
        e[0][2] * e[1][1] * e[2][0] * e[3][3] + e[0][1] * e[1][2] * e[2][0] * e[3][3] +
        e[0][2] * e[1][0] * e[2][1] * e[3][3] - e[0][0] * e[1][2] * e[2][1] * e[3][3] -
        e[0][1] * e[1][0] * e[2][2] * e[3][3] + e[0][0] * e[1][1] * e[2][2] * e[3][3]
    );
}

inline M4
M4::minors() const noexcept
{
    auto& e = m_a;
    return {
        M3(e[1][1], e[1][2], e[1][3],    e[2][1], e[2][2], e[2][3],    e[3][1], e[3][2], e[3][3]).det(),
        M3(e[1][0], e[1][2], e[1][3],    e[2][0], e[2][2], e[2][3],    e[3][0], e[3][2], e[3][3]).det(),
        M3(e[1][0], e[1][1], e[1][3],    e[2][0], e[2][1], e[2][3],    e[3][0], e[3][1], e[3][3]).det(),
        M3(e[1][0], e[1][1], e[1][2],    e[2][0], e[2][1], e[2][2],    e[3][0], e[3][1], e[3][2]).det(),

        M3(e[0][1], e[0][2], e[0][3],    e[2][1], e[2][2], e[2][3],    e[3][1], e[3][2], e[3][3]).det(),
        M3(e[0][0], e[0][2], e[0][3],    e[2][0], e[2][2], e[2][3],    e[3][0], e[3][2], e[3][3]).det(),
        M3(e[0][0], e[0][1], e[0][3],    e[2][0], e[2][1], e[2][3],    e[3][0], e[3][1], e[3][3]).det(),
        M3(e[0][0], e[0][1], e[0][2],    e[2][0], e[2][1], e[2][2],    e[3][0], e[3][1], e[3][2]).det(),

        M3(e[0][1], e[0][2], e[0][3],    e[1][1], e[1][2], e[1][3],    e[3][1], e[3][2], e[3][3]).det(),
        M3(e[0][0], e[0][2], e[0][3],    e[1][0], e[1][2], e[1][3],    e[3][0], e[3][2], e[3][3]).det(),
        M3(e[0][0], e[0][1], e[0][3],    e[1][0], e[1][1], e[1][3],    e[3][0], e[3][1], e[3][3]).det(),
        M3(e[0][0], e[0][1], e[0][2],    e[1][0], e[1][1], e[1][2],    e[3][0], e[3][1], e[3][2]).det(),

        M3(e[0][1], e[0][2], e[0][3],    e[1][1], e[1][2], e[1][3],    e[2][1], e[2][2], e[2][3]).det(),
        M3(e[0][0], e[0][2], e[0][3],    e[1][0], e[1][2], e[1][3],    e[2][0], e[2][2], e[2][3]).det(),
        M3(e[0][0], e[0][1], e[0][3],    e[1][0], e[1][1], e[1][3],    e[2][0], e[2][1], e[2][3]).det(),
        M3(e[0][0], e[0][1], e[0][2],    e[1][0], e[1][1], e[1][2],    e[2][0], e[2][1], e[2][2]).det()
    };
}

inline M4
M4::cofactors() const noexcept
{
    M4 m = minors();

    V4 plusMinus{+1, -1, +1, -1};
    V4 minusPlus{-1, +1, -1, +1};

    m[0] *= plusMinus;
    m[1] *= minusPlus;
    m[2] *= plusMinus;
    m[3] *= minusPlus;

    return m;
}

inline M4
M4::transposed() const noexcept
{
    auto& e = m_a;
    return {
        e[0][0], e[1][0], e[2][0], e[3][0],
        e[0][1], e[1][1], e[2][1], e[3][1],
        e[0][2], e[1][2], e[2][2], e[3][2],
        e[0][3], e[1][3], e[2][3], e[3][3]
    };
}

inline M4
M4::adj() const noexcept
{
    return cofactors().transposed();
}

inline M4
M4::inv() const noexcept
{
    return (1.0 / det()) * adj();
}

inline M4
M4::translated(const V3& tv) const noexcept
{
    return *this * translationFrom(tv);
}

inline M4
M4::scaled(const f32 s) const noexcept
{
    return *this * scaledFrom(s);
}

inline M4
M4::scaled(const V3& s) const noexcept
{
    return *this * scaledFrom(s);
}

inline M4
M4::rotated(const f32 th, const V3& ax) const noexcept
{
    return *this * rotFrom(th, ax);
}

inline M4
M4::rotatedX(const f32 th) const noexcept
{
    return *this * rotXFrom(th);
}

inline M4
M4::rotatedY(const f32 th) const noexcept
{
    return *this * rotYFrom(th);
}

inline M4
M4::rotatedZ(const f32 th) const noexcept
{
    return *this * rotZFrom(th);
}

/* static */ inline M4
M4::scaledFrom(const f32 s) noexcept
{
    return {
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    };
}

/* static */ inline M4
M4::scaledFrom(const V3& v) noexcept
{
    return {
        v.x(), 0,     0,     0,
        0,     v.y(), 0,     0,
        0,     0,     v.z(), 0,
        0,     0,     0,     1
    };
}

/* static */ inline M4
M4::scaledFrom(f32 x, f32 y, f32 z) noexcept
{
    return scaledFrom(V3{x, y, z});
}

/* static */ inline M4
M4::persFrom(const f32 fov, const f32 asp, const f32 n, const f32 f) noexcept
{
    M4 res {};
    res[0].x() = 1.0f / (asp * std::tan(fov * 0.5f));
    res[1].y() = 1.0f / (std::tan(fov * 0.5f));
    res[2].z() = -f / (n - f);
    res[3].z() = n * f / (n - f);
    res[2].w() = 1.0f;

    return res;
}

/* static */ inline M4
M4::orthoFrom(const f32 l, const f32 r, const f32 b, const f32 t, const f32 n, const f32 f) noexcept
{
    return {
        2/(r-l),       0,            0,           0,
        0,             2/(t-b),      0,           0,
        0,             0,           -2/(f-n),     0,
        -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1
    };
}

/* static */ inline M4
M4::lookAtFrom(const V3& R, const V3& U, const V3& D, const V3& P) noexcept
{
    M4 m0 {
        R.x(),  U.x(),  D.x(),  0,
        R.y(),  U.y(),  D.y(),  0,
        R.z(),  U.z(),  D.z(),  0,
        0,      0,      0,      1
    };

    return m0.translated({-P.x(), -P.y(), -P.z()});
}

/* static */ inline M4
M4::lookAtFrom(const V3& eyeV, const V3& centerV, const V3& upV) noexcept
{
    V3 camDir = (eyeV - centerV).normalized();
    V3 camRight = upV.cross(camDir).normalized();
    V3 camUp = camDir.cross(camRight);

    return lookAtFrom(camDir, camUp, camDir, eyeV);
}

/* static */ inline M4
M4::rotFrom(const f32 th, const V3& ax) noexcept
{
    const f32 c = std::cos(th);
    const f32 s = std::sin(th);

    const f32 x = ax.x();
    const f32 y = ax.y();
    const f32 z = ax.z();

    return {
        ((1 - c)*sq(x)) + c, ((1 - c)*x*y) - s*z, ((1 - c)*x*z) + s*y, 0,
        ((1 - c)*x*y) + s*z, ((1 - c)*sq(y)) + c, ((1 - c)*y*z) - s*x, 0,
        ((1 - c)*x*z) - s*y, ((1 - c)*y*z) + s*x, ((1 - c)*sq(z)) + c, 0,
        0,                   0,                   0,                   1
    };
}

/* static */ inline M4
M4::rotXFrom(const f32 th) noexcept
{
    return {
        1,  0,            0,            0,
        0,  std::cos(th), std::sin(th), 0,
        0, -std::sin(th), std::cos(th), 0,
        0,  0,            0,            1
    };
}

/* static */ inline M4
M4::rotYFrom(const f32 th) noexcept
{
    return {
        std::cos(th), 0,  std::sin(th),  0,
        0,            1,  0,             0,
       -std::sin(th), 0,  std::cos(th),  0,
        0,            0,  0,             1
    };
}

/* static */ inline M4
M4::rotZFrom(const f32 th) noexcept
{
    return {
        std::cos(th),  std::sin(th), 0, 0,
       -std::sin(th),  std::cos(th), 0, 0,
        0,             0,            1, 0,
        0,             0,            0, 1
    };
}

/* static */ inline M4
M4::rotFrom(const f32 x, const f32 y, const f32 z) noexcept
{
    return rotZFrom(z) * rotYFrom(y) * rotXFrom(x);
}

/* static */ inline M4
M4::translationFrom(const V3& tv) noexcept
{
    return {
        1,      0,      0,      0,
        0,      1,      0,      0,
        0,      0,      1,      0,
        tv.x(), tv.y(), tv.z(), 1
    };
}

/* static */ inline M4
M4::translationFrom(const f32 x, const f32 y, const f32 z) noexcept
{
    return translationFrom(V3{x, y, z});
}

inline
Qt::Qt(int) noexcept
    : V4{0, 0, 0, 1}
{
}

inline
Qt::Qt(V4 v4) noexcept
    : V4{v4}
{
}

inline Qt
Qt::operator-() const noexcept
{
    Qt r {UNINIT};
    static_cast<V2&>(r) = -static_cast<const V2&>(*this);
    return r;
}

inline Qt
Qt::operator*(const Qt& r) const noexcept
{
    auto& l = *this;

    return {
        l.w()*r.x() + l.x()*r.w() + l.y()*r.z() - l.z()*r.y(),
        l.w()*r.y() - l.x()*r.z() + l.y()*r.w() + l.z()*r.x(),
        l.w()*r.z() + l.x()*r.y() - l.y()*r.x() + l.z()*r.w(),
        l.w()*r.w() - l.x()*r.x() - l.y()*r.y() - l.z()*r.z(),
    };
}

inline Qt
Qt::operator*(const V4& r) const noexcept
{
    return *this * ((const Qt&)r);
}

inline Qt
Qt::operator*=(const Qt& r) noexcept
{
    return *this = *this * r;
}

inline Qt
Qt::operator*=(const V4& r) noexcept
{
    return *this = *this * r;
}

inline M4
Qt::rot() const noexcept
{
    auto& x = this->x();
    auto& y = this->y();
    auto& z = this->z();
    auto& w = this->w();

    return {
        1 - 2*y*y - 2*z*z, 2*x*y + 2*w*z,     2*x*z - 2*w*y,     0,
        2*x*y - 2*w*z,     1 - 2*x*x - 2*z*z, 2*y*z + 2*w*x,     0,
        2*x*z + 2*w*y,     2*y*z - 2*w*x,     1 - 2*x*x - 2*y*y, 0,
        0,                 0,                 0,                 1
    };
}

inline M4
Qt::rot2() const noexcept
{
    f32 x = this->x(), y = this->y(), z = this->z(), w = this->w();

    f32 xx = x * x;
    f32 yy = y * y;
    f32 zz = z * z;
    f32 xy = x * y;
    f32 xz = x * z;
    f32 yz = y * z;
    f32 wx = w * x;
    f32 wy = w * y;
    f32 wz = w * z;

    M4 m {UNINIT};
    m[0][0] = 1.0f - 2.0f * (yy + zz);
    m[0][1] = 2.0f * (xy + wz);
    m[0][2] = 2.0f * (xz - wy);
    m[0][3] = 0.0f;

    m[1][0] = 2.0f * (xy - wz);
    m[1][1] = 1.0f - 2.0f * (xx + zz);
    m[1][2] = 2.0f * (yz + wx);
    m[1][3] = 0.0f;

    m[2][0] = 2.0f * (xz + wy);
    m[2][1] = 2.0f * (yz - wx);
    m[2][2] = 1.0f - 2.0f * (xx + yy);
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;

    return m;
}

inline Qt
Qt::conj() const noexcept
{
    return {x(), y(), z(), w()};
}

inline Qt
Qt::normalized() const noexcept
{
    f32 mag = std::sqrt(w() * w() + x() * x() + y() * y() + z() * z());
    return {x() / mag, y() / mag, z() / mag, w() / mag};
}

/* static */ inline Qt
Qt::axisAngleFrom(const V3& axis, f32 th) noexcept
{
    f32 sinTh = std::sin(th / 2.0f);

    return {
        axis.x() * sinTh,
        axis.y() * sinTh,
        axis.z() * sinTh,
        std::cos(th / 2.0f)
    };
}

inline Qt
slerp(const Qt& q1, const Qt& q2, f32 t) noexcept
{
    auto dot = q1.dot(q2);

    Qt q2b = q2;
    if (dot < 0.0f)
    {
        q2b = -q2b;
        dot = -dot;
    }

#if defined ADT_SSE4_2 && 0

    if (dot > 0.9995f)
    {
        auto q1Pack = simd::f32x4(q1.base);

        auto diff = simd::f32x4(q2b.base) - q1Pack;
        auto mul = diff * t;
        auto sum = q1Pack + mul;

        return QtNorm(Qt(sum));
    }

#else

    if (dot > 0.9995f)
    {
        Qt res {
            q1.x() + t * (q2b.x() - q1.x()),
            q1.y() + t * (q2b.y() - q1.y()),
            q1.z() + t * (q2b.z() - q1.z()),
            q1.w() + t * (q2b.w() - q1.w())
        };
        return res.normalized();
    }

#endif

    f32 theta0 = std::acos(dot);
    f32 theta = theta0 * t;

    f32 sinTheta0 = std::sin(theta0);
    f32 sinTheta = std::sin(theta);

    f32 s1 = std::cos(theta) - dot * (sinTheta / sinTheta0);
    f32 s2 = sinTheta / sinTheta0;

#if defined ADT_SSE4_2 && 0

    auto res = V4((simd::f32x4(q1.base) * s1) + (simd::f32x4(q2b.base) * s2));
    return Qt(res);

#else

    Qt res {UNINIT};

    res.x() = (s1 * q1.x()) + (s2 * q2b.x());
    res.y() = (s1 * q1.y()) + (s2 * q2b.y());
    res.z() = (s1 * q1.z()) + (s2 * q2b.z());
    res.w() = (s1 * q1.w()) + (s2 * q2b.w());

    return res;

#endif

}

inline M4
transformation(const V3& translation, const Qt& rot, const V3& scale) noexcept
{
    return M4::translationFrom(translation) * rot.rot() * M4::scaledFrom(scale);
}

inline M4
transformation(const V3& translation, const V3& scale) noexcept
{
    return M4::translationFrom(translation) * M4::scaledFrom(scale);
}

} /* namespace adt::math */

namespace adt::print
{

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::V2Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y());
}

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::V3Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y(), x.z());
}

template<typename T>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::V4Base<T>& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.x(), x.y(), x.z(), x.w());
}

template<>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::M2& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\t(", x[0][0], ", ", x[0][1],
        "\n\t ", x[1][0], ", ", x[1][1], ")"
    );
}

template<>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::M3& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\t(", x[0][0], ", ", x[0][1], ", ", x[0][2],
        "\n\t ", x[1][0], ", ", x[1][1], ", ", x[1][2],
        "\n\t ", x[2][0], ", ", x[2][1], ", ", x[2][2], ")"
    );
}

template<>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::M4& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\t(", x[0][0], ", ", x[0][1], ", ", x[0][2], ", ", x[0][3],
        "\n\t ", x[1][0], ", ", x[1][1], ", ", x[1][2], ", ", x[1][3],
        "\n\t ", x[2][0], ", ", x[2][1], ", ", x[2][2], ", ", x[2][3],
        "\n\t ", x[3][0], ", ", x[3][1], ", ", x[3][2], ", ", x[3][3], ")"
    );
}

template<>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const math::Qt& x)
{
    return format(pCtx, fmtArgs, static_cast<const math::V4&>(x));
}

} /* namespace adt::print */
