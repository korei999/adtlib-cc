#pragma once

#include "adt/types.hh"
#include "adt/print-inl.hh"

#include <limits>

namespace adt::math2
{

constexpr f64 PI64 = 3.14159265358979323846;
constexpr f32 PI32 = static_cast<f32>(PI64);
constexpr f64 EPS64 = std::numeric_limits<f64>::epsilon();
constexpr f32 EPS32 = std::numeric_limits<f32>::epsilon();

constexpr inline f64 toDeg(f64 x);
constexpr inline f64 toRad(f64 x);
constexpr inline f32 toDeg(f32 x);
constexpr inline f32 toRad(f32 x);

constexpr inline f64 toRad(i64 x);
constexpr inline f64 toDeg(i64 x);
constexpr inline f32 toRad(i32 x);
constexpr inline f32 toDeg(i32 x);

inline bool eq(const f64 l, const f64 r);
inline bool eq(const f32 l, const f32 r);

constexpr inline auto sq(const auto& x);
constexpr inline auto cube(const auto& x);

constexpr inline i64 sign(i64 x);

template<typename T>
struct V2Base
{
    T m_a[2] {};

    /* */

    V2Base() = default;
    V2Base(T _x, T _y) noexcept : m_a{_x, _y} {}

    /* */

    template<typename B> explicit operator V2Base<B>() const noexcept { return {static_cast<B>(x()), static_cast<B>(y())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return s.m_a[i]; }

    template<typename S> decltype(auto) x(this S&& s) noexcept { return s.m_a[0]; }
    template<typename S> decltype(auto) y(this S&& s) noexcept { return s.m_a[1]; }
    template<typename S> decltype(auto) xy(this S&& s) noexcept { return s; }

    bool operator==(const V2Base& r) const noexcept;
    V2Base operator-() noexcept;
    V2Base operator+(const V2Base& r) const noexcept;
    V2Base operator-(const V2Base& r) const noexcept;
    V2Base operator*(f32 s) const noexcept;
    friend V2Base operator*(f32 s, const V2Base& v) noexcept { return v * s; }
    V2Base operator*(const V2Base& r) const noexcept;
    V2Base& operator*=(const V2Base& r) noexcept;
    V2Base operator/(f32 s) const noexcept;
    V2Base& operator+=(const V2Base& r) noexcept;
    V2Base& operator-=(const V2Base& r) noexcept;
    V2Base& operator*=(f32 r) noexcept;
    V2Base& operator/=(f32 r) noexcept;
};

using V2 = V2Base<f32>;
using IV2 = V2Base<i32>;

template<typename T>
struct V3Base : V2Base<T>
{
    using V2B = V2Base<T>;
    using V2B::x;
    using V2B::y;
    using V2B::xy;

    /* */

    T m_z {};

    /* */

    V3Base() = default;
    V3Base(T _x, T _y, T _z) noexcept : V2B{_x, _y}, m_z{_z} {}
    V3Base(V2B _xy, T _z) noexcept : V2B{_xy}, m_z{_z} {}
    V3Base(T _x, V2B _yz) noexcept : V2B{_x, _yz.x()}, m_z{_yz.y()} {}

    /* */

    template<typename B> explicit operator V3Base<B>() const noexcept { return {static_cast<B>(x()), static_cast<B>(y()), static_cast<B>(z())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((f32(&)[3])(s))[i]; }

    template<typename S> decltype(auto) z(this S&& s) noexcept { return s.m_z; }
    template<typename S> decltype(auto) xyz(this S&& s) noexcept { return s; }

    V2B yz() const noexcept { return {y(), z()}; }

    bool operator==(const V3Base& r) const noexcept;

    V3Base operator+(const V3Base& r) const noexcept;
    V3Base operator-(const V3Base& r) const noexcept;
    V3Base operator-() const noexcept;
    V3Base operator*(f32 s) const noexcept;
    friend V3Base operator*(f32 s, const V3Base& v) noexcept { return v * s; }
    V3Base operator*(const V3Base& r) const noexcept;
    V3Base operator/(f32 s) const noexcept;
    V3Base operator+(f32 b) const noexcept;
    V3Base& operator+=(f32 b) noexcept;
    V3Base& operator+=(const V3Base& r) noexcept;
    V3Base& operator-=(const V3Base& r) noexcept;
    V3Base& operator*=(f32 s) noexcept;
    V3Base& operator/=(f32 s) noexcept;
};

using V3 = V3Base<f32>;
using IV3 = V3Base<i32>;

template<typename T>
struct V4Base : V3Base<T>
{
    using V3B = V3Base<T>;
    using V2B = V2Base<T>;
    using V3B::x;
    using V3B::y;
    using V3B::z;
    using V3B::xy;
    using V3B::yz;

    /* */

    T m_w {};

    /* */

    V4Base() = default;
    V4Base(T _x, T _y, T _z, T _w) noexcept : V3B{_x, _y, _z}, m_w{_w} {}
    V4Base(V3B _xyz, T _w) noexcept : V3B{_xyz}, m_w{_w} {}
    V4Base(T _x, V3B _yzw) noexcept : V3B{_x, _yzw.x(), _yzw.y()}, m_w{_yzw.z()} {}
    V4Base(V2B _xy, T _z, T _w) noexcept : V3B{_xy, _z}, m_w{_w} {}
    V4Base(T _x, T _y, V2B _zw) noexcept : V3B{_x, _y, _zw.x()}, m_w{_zw.y()} {}
    V4Base(V2B _xy, V2B _zw) noexcept : V3B{_xy, _zw.x()}, m_w{_zw.y()} {}
    V4Base(T _x, V2B _yz, T _w) noexcept : V3B{_x, _yz.x(), _yz.y()}, m_w{_w} {}

    /* */

    template<typename S> decltype(auto) w(this S&& s) noexcept { return s.m_w; }
    template<typename S> decltype(auto) xyzw(this S&& s) noexcept { return s; }

    V2B zw() const noexcept { return {y(), z()}; }
    V3B yzw() const noexcept { return {y(), z(), w()}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((f32(&)[4])(s))[i]; }

    template<typename B> explicit operator V4Base<B>() const noexcept
    { return {static_cast<B>(x()), static_cast<B>(y()), static_cast<B>(z()), static_cast<B>(w())}; }
};

using V4 = V4Base<f32>;
using IV4 = V4Base<f32>;

} /* namespace adt::math2 */

namespace adt::print
{

template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V2Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V3Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V4Base<T>& x);

} /* namespace adt::print */
