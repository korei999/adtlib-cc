#pragma once

#include "adt/print-inl.hh"

#include <limits>

namespace adt::math
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

template<typename T, std::floating_point F>
constexpr T
bezier(
    const T& p0,
    const T& p1,
    const T& p2,
    const F t
);

template<typename T, std::floating_point F>
constexpr T
bezier(
    const T& p0,
    const T& p1,
    const T& p2,
    const T& p3,
    const F t
);

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

    template<typename S> decltype(auto) data(this S&& s) noexcept { return (T(&)[2])(s); }

    template<typename B> explicit operator V2Base<B>() const noexcept { return {static_cast<B>(x()), static_cast<B>(y())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return s.m_a[i]; }

    template<typename S> constexpr decltype(auto) x(this S&& s) noexcept { return s.m_a[0]; }
    template<typename S> constexpr decltype(auto) y(this S&& s) noexcept { return s.m_a[1]; }

    template<typename S> constexpr decltype(auto) r(this S&& s) noexcept { return s.m_a[0]; }
    template<typename S> constexpr decltype(auto) g(this S&& s) noexcept { return s.m_a[1]; }

    template<typename S> constexpr decltype(auto) u(this S&& s) noexcept { return s.m_a[0]; }
    template<typename S> constexpr decltype(auto) v(this S&& s) noexcept { return s.m_a[1]; }

    bool operator==(const V2Base& r) const noexcept;
    V2Base operator-() const noexcept;
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

    template<typename S> decltype(auto) data(this S&& s) noexcept { return (T(&)[3])(s); }

    template<typename B> explicit operator V3Base<B>() const noexcept
    { return {static_cast<B>(x()), static_cast<B>(y()), static_cast<B>(z())}; }

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((T(&)[3])(s))[i]; }
    constexpr T& z() noexcept { return m_z; }
    constexpr const T& z() const noexcept { return m_z; }

    constexpr T& b() noexcept { return m_z; }
    constexpr const T& b() const noexcept { return m_z; }

    V2B xy() const noexcept { return *static_cast<const V2B*>(this); }
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
    constexpr V4Base(T _x, V2B _yz, T _w) noexcept : V3B{_x, _yz}, m_w{_w} {}
    constexpr V4Base(T(&a)[4]) noexcept : V3B{a[0], a[1], a[2]}, m_w{a[3]} {}

    /* */

    template<typename S> decltype(auto) data(this S&& s) noexcept { return (T(&)[4])(s); }

    constexpr T& w() noexcept { return m_w; }
    constexpr const T& w() const noexcept { return m_w; }

    constexpr T& a() noexcept { return m_w; }
    constexpr const T& a() const noexcept { return m_w; }

    V3B& xyz() noexcept { return *static_cast<V3B*>(this); }
    const V3B& xyz() const noexcept { return *static_cast<const V3B*>(this); }

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

struct M2
{
    f32 m_a[2][2];

    /* */

    M2() noexcept : m_a{} {}
    explicit M2(UninitFlag) noexcept {}
    explicit M2(int) noexcept;
    M2(f32 _0, f32 _1, f32 _2, f32 _3) noexcept : m_a{_0, _1, _2, _3} {}

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((V2(&)[2])(s))[i]; }
    template<typename S> decltype(auto) data(this S&& s) noexcept { return (f32(&)[2*2])(s); }

    f32 det() const noexcept;
};

struct M3
{
    f32 m_a[3][3];

    /* */

    constexpr M3() noexcept : m_a{} {}
    explicit constexpr M3(int) noexcept;
    explicit constexpr M3(UninitFlag) noexcept {}
    constexpr M3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8) noexcept;
    constexpr M3(const V3& _0, const V3& _1, const V3& _2) noexcept;

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((V3(&)[3])(s))[i]; }
    template<typename S> decltype(auto) data(this S&& s) noexcept { return (f32(&)[3*3])(s); }

    f32 det() const noexcept;
    M3 minors() const noexcept;
    M3 cofactors() const noexcept;
    M3 transposed() const noexcept;
    M3 adj() const noexcept;
    M3 inv() const noexcept;
    M3 normal() const noexcept;
    M3 scaled(const f32 s) const noexcept;
    M3 scaled(const V2 s) const noexcept;

    bool operator==(const M3& r) const noexcept;

    M3 operator*(const f32 r) const noexcept;
    M3& operator*=(const f32 r) noexcept;
    friend M3 operator*(const f32 l, const M3& r) noexcept { return r * l; }
    V3 operator*(const V3& r) const noexcept;
    M3 operator*(const M3& r) const noexcept;
    M3& operator*=(const M3& r) noexcept;
};

struct M4
{
    f32 m_a[4][4];

    /* */

    constexpr M4() noexcept : m_a{} {}
    explicit constexpr M4(UninitFlag) noexcept {};
    explicit constexpr M4(int) noexcept;
    constexpr M4(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8, f32 _9, f32 _10, f32 _11, f32 _12, f32 _13, f32 _14, f32 _15) noexcept;
    constexpr M4(const V4& _0, const V4& _1, const V4& _2, const V4& _3) noexcept;

    /* */

    template<typename S> decltype(auto) operator[](this S&& s, int i) noexcept { return ((V4(&)[4])(s))[i]; }
    template<typename S> decltype(auto) data(this S&& s) noexcept { return ((f32(&)[4*4])(s)); }

    bool operator==(const M4& r) const noexcept;

    M4 operator*(const f32 r) const noexcept;
    M4 operator*(bool) = delete;
    M4& operator*=(const f32 r) noexcept;
    M4& operator*=(bool) = delete;
    friend M4 operator*(const f32 l, const M4& r) noexcept { return r * l; }
    V4 operator*(const V4& r) const noexcept;
    M4 operator*(const M4& r) const noexcept;
    M4& operator*=(const M4& r) noexcept;

    f32 det() const noexcept;
    M4 minors() const noexcept;
    M4 cofactors() const noexcept;
    M4 transposed() const noexcept;
    M4 adj() const noexcept;
    M4 inv() const noexcept;
    M4 translated(const V3& tv) const noexcept;
    M4 scaled(const f32 s) const noexcept;
    M4 scaled(const V3& s) const noexcept;
    M4 rotated(const f32 th, const V3& ax) const noexcept;
    M4 rotatedX(const f32 th) const noexcept;
    M4 rotatedY(const f32 th) const noexcept;
    M4 rotatedZ(const f32 th) const noexcept;

    static M4 translationFrom(const V3& tv) noexcept;
    static M4 translationFrom(const f32 x, const f32 y, const f32 z) noexcept;
    static M4 scaledFrom(const f32 s) noexcept;
    static M4 scaledFrom(const V3& v) noexcept;
    static M4 scaledFrom(f32 x, f32 y, f32 z) noexcept;
    static M4 persFrom(const f32 fov, const f32 asp, const f32 n, const f32 f) noexcept;
    static M4 orthoFrom(const f32 l, const f32 r, const f32 b, const f32 t, const f32 n, const f32 f) noexcept;
    static M4 lookAtFrom(const V3& R, const V3& U, const V3& D, const V3& P) noexcept;
    static M4 lookAtFrom(const V3& eyeV, const V3& centerV, const V3& upV) noexcept;
    static M4 rotFrom(const f32 th, const V3& ax) noexcept;
    static M4 rotXFrom(const f32 th) noexcept;
    static M4 rotYFrom(const f32 th) noexcept;
    static M4 rotZFrom(const f32 th) noexcept;
    static M4 rotFrom(const f32 x, const f32 y, const f32 z) noexcept;
};

struct Qt : V4
{
    using V4::V4Base;

    /* */

    Qt() = default;
    Qt(int) noexcept;
    explicit Qt(V4 v4) noexcept;
    explicit Qt(UninitFlag) noexcept {}

    /* */

    Qt operator-() const noexcept;
    Qt operator*(const Qt& r) const noexcept;
    Qt operator*(const V4& r) const noexcept;
    Qt operator*=(const Qt& r) noexcept;
    Qt operator*=(const V4& r) noexcept;

    Qt flipped() const noexcept { return {w(), z(), y(), x()}; }

    M4 rot() const noexcept;
    M4 rot2() const noexcept;
    Qt conj() const noexcept;
    Qt normalized() const noexcept;

    static Qt axisAngleFrom(const V3& axis, f32 th) noexcept;
};

inline Qt slerp(const Qt& q1, const Qt& q2, f32 t) noexcept;

inline M4 transformation(const V3& translation, const Qt& rot, const V3& scale) noexcept;
inline M4 transformation(const V3& translation, const V3& scale) noexcept;

} /* namespace adt::math */

namespace adt::print
{

template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::V2Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::V3Base<T>& x);
template<typename T> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::V4Base<T>& x);
template<> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::M2& x);
template<> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::M3& x);
template<> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::M4& x);
template<> inline isize format(Context* pCtx, FormatArgs fmtArgs, const math::Qt& x);

} /* namespace adt::print */
