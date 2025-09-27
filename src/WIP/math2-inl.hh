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

template<typename T> inline bool eq(const T& l, const T& r);
template<> inline bool eq(const f64& l, const f64& r);
template<> inline bool eq(const f32& l, const f32& r);

constexpr inline auto sq(const auto& x);
constexpr inline auto cube(const auto& x);

constexpr inline i64 sign(i64 x);

constexpr inline auto lerp(const auto& a, const auto& b, const auto& t);

template<typename T>
struct V2Base
{
    T m_a[2];

    /* */

    constexpr V2Base() noexcept : m_a{} {}
    constexpr explicit V2Base(UninitFlag) noexcept {}
    constexpr V2Base(T _x, T _y) noexcept : m_a{_x, _y} {}
    constexpr V2Base(T(&a)[2]) noexcept : m_a{a[0], a[1]} {}

    /* */

    template<typename B> explicit operator V2Base<B>() const noexcept { return {static_cast<B>(x()), static_cast<B>(y())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return s.m_a[i]; }

    template<typename S> decltype(auto) x(this S&& s) noexcept { return s.m_a[0]; }
    template<typename S> decltype(auto) y(this S&& s) noexcept { return s.m_a[1]; }

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

    T length() const noexcept;
    V2Base normalized() const noexcept;
    V2Base clamped(const V2Base& min, const V2Base& max) const noexcept;
    T dot(const V2Base& r) const noexcept;
    T dist(const V2Base& r) const noexcept;
    T cross(const V2Base& r) const noexcept;
};

using V2 = V2Base<f32>;
using IV2 = V2Base<i32>;

template<typename T>
struct V3Base : V2Base<T>
{
    using V2B = V2Base<T>;
    using V2B::x;
    using V2B::y;

    /* */

    T m_z;

    /* */

    constexpr V3Base() noexcept : m_z {} {}
    constexpr explicit V3Base(UninitFlag) noexcept {}
    constexpr V3Base(T _x, T _y, T _z) noexcept : V2B{_x, _y}, m_z{_z} {}
    constexpr V3Base(V2B _xy, T _z) noexcept : V2B{_xy}, m_z{_z} {}
    constexpr V3Base(T _x, V2B _yz) noexcept : V2B{_x, _yz.x()}, m_z{_yz.y()} {}
    constexpr V3Base(T(&a)[3]) noexcept : V2B{a[0], a[1]}, m_z{a[2]} {}

    /* */

    template<typename B> explicit operator V3Base<B>() const noexcept
    { return {static_cast<B>(x()), static_cast<B>(y()), static_cast<B>(z())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[3])(s))[i]; }

    template<typename S> decltype(auto) z(this S&& s) noexcept { return s.m_z; }

    V2B xy() const noexcept { return *static_cast<V2B*>(this); }
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

    T length() const noexcept;
    V3Base normalized(const f32 len) const noexcept;
    V3Base normalized() const noexcept;
    T dot(const V3Base& r) const noexcept;
    T rad(const V3Base& r) const noexcept;
    T dist(const V3Base& r) const noexcept;
    V3Base cross(const V3Base& r) const noexcept;
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

    constexpr V4Base() = default;
    constexpr V4Base(T _x, T _y, T _z, T _w) noexcept : V3B{_x, _y, _z}, m_w{_w} {}
    constexpr V4Base(V3B _xyz, T _w) noexcept : V3B{_xyz}, m_w{_w} {}
    constexpr V4Base(T _x, V3B _yzw) noexcept : V3B{_x, _yzw.x(), _yzw.y()}, m_w{_yzw.z()} {}
    constexpr V4Base(V2B _xy, T _z, T _w) noexcept : V3B{_xy, _z}, m_w{_w} {}
    constexpr V4Base(T _x, T _y, V2B _zw) noexcept : V3B{_x, _y, _zw.x()}, m_w{_zw.y()} {}
    constexpr V4Base(V2B _xy, V2B _zw) noexcept : V3B{_xy, _zw.x()}, m_w{_zw.y()} {}
    constexpr V4Base(T _x, V2B _yz, T _w) noexcept : V3B{_x, _yz.x(), _yz.y()}, m_w{_w} {}
    constexpr V4Base(T(&a)[4]) noexcept : V3B{a[0], a[1], a[2]}, m_w{a[3]} {}

    /* */

    template<typename S> decltype(auto) w(this S&& s) noexcept { return s.m_w; }

    V3B xyz() const noexcept { return *static_cast<const V3B*>(this); }
    V2B zw() const noexcept { return {y(), z()}; }
    V3B yzw() const noexcept { return {y(), z(), w()}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[4])(s))[i]; }

    template<typename B> explicit operator V4Base<B>() const noexcept
    { return {static_cast<B>(x()), static_cast<B>(y()), static_cast<B>(z()), static_cast<B>(w())}; }

    bool operator==(const V4Base& r) const noexcept;

    V4Base operator+(const V4Base& r) const noexcept;
    V4Base operator-() const noexcept;
    V4Base operator-(const V4Base& r) const noexcept;
    V4Base operator*(f32 r) const noexcept;
    friend V4Base operator*(f32 l, const V4Base& r) noexcept { return r * l; }
    V4Base operator*(const V4Base& r) const noexcept;
    V4Base& operator*=(const V4Base& r) noexcept;
    V4Base operator/(f32 r) const noexcept;
    friend V4Base operator/(f32 l, const V4Base& r) noexcept { return r * l; }
    V4Base& operator+=(const V4Base& r) noexcept;
    V4Base& operator-=(const V4Base& r) noexcept;
    V4Base& operator*=(f32 r) noexcept;
    V4Base& operator/=(f32 r) noexcept;

    T length() const noexcept;
    V4Base normalized() const noexcept;
    T dot(const V4Base& r) const noexcept;
};

using V4 = V4Base<f32>;
using IV4 = V4Base<f32>;

template<typename T>
struct M2Base
{
    T m_a[2][2] {};

    /* */

    M2Base() = default;
    M2Base(InitFlag) noexcept;
    M2Base(T _0, T _1, T _2, T _3) noexcept : m_a{_0, _1, _2, _3} {}

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[2*2])(s))[i]; }
    template<typename S> decltype(auto) operator[](this S&& s, int i, int j) noexcept { return s.m_a[i][j]; }

    template<typename S> decltype(auto) data(this S&& s) noexcept { return ((T(&)[2*2])(s)); }
    template<typename S> decltype(auto) v2s(this S&& s) noexcept { return ((V2Base<T>(&)[2])(s)); }

    T det() const noexcept;
};

using M2 = M2Base<f32>;

template<typename T>
struct M3Base
{
    T m_a[3][3];

    /* */

    constexpr M3Base() noexcept : m_a{} {}
    explicit constexpr M3Base(int) noexcept;
    explicit constexpr M3Base(UninitFlag) noexcept {}
    constexpr M3Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8) noexcept;

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[3*3])(s))[i]; }
    template<typename S> decltype(auto) operator[](this S&& s, int i, int j) noexcept { return s.m_a[i][j]; }

    template<typename S> decltype(auto) data(this S&& s) noexcept { return ((T(&)[3*3])(s)); }
    template<typename S> decltype(auto) v3s(this S&& s) noexcept { return ((V3Base<T>(&)[3])(s)); }

    T det() const noexcept;
    M3Base minors() const noexcept;
    M3Base cofactors() const noexcept;
    M3Base transposed() const noexcept;
    M3Base adj() const noexcept;
    M3Base inv() const noexcept;
    M3Base normal() const noexcept;
    M3Base scaled(const f32 s) const noexcept;
    M3Base scaled(const V2Base<T> s) const noexcept;

    bool operator==(const M3Base& r) const noexcept;

    M3Base operator*(const f32 r) const noexcept;
    M3Base& operator*=(const f32 r) noexcept;
    friend M3Base operator*(const f32 l, const M3Base& r) noexcept { return r * l; }
    V3Base<T> operator*(const V3Base<T>& r) const noexcept;
    M3Base operator*(const M3Base& r) const noexcept;
    M3Base& operator*=(const M3Base& r) noexcept;
};

using M3 = M3Base<f32>;

template<typename T>
struct M4Base
{
    T m_a[4][4];

    /* */

    constexpr M4Base() noexcept : m_a{} {}
    explicit constexpr M4Base(UninitFlag) noexcept {};
    explicit constexpr M4Base(int) noexcept;
    constexpr M4Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8, T _9, T _10, T _11, T _12, T _13, T _14, T _15) noexcept;

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[4*4])(s))[i]; }
    template<typename S> decltype(auto) operator[](this S&& s, int i, int j) noexcept { return s.m_a[i][j]; }

    template<typename S> decltype(auto) data(this S&& s) noexcept { return ((T(&)[4*4])(s)); }
    template<typename S> decltype(auto) v4s(this S&& s) noexcept { return ((V4Base<T>(&)[4])(s)); }
};

using M4 = M4Base<f32>;

} /* namespace adt::math2 */

namespace adt::print
{

template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V2Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V3Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::V4Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::M2Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math2::M3Base<T>& x);

} /* namespace adt::print */
