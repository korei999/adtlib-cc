#pragma once

#include "adt/logs.hh"

struct What
{
    int* p {};

    What() : p {new int {0}} {}
    What(int i) : p {new int {i}} {}
    What(const What& r) : p {r.p ? new int {*r.p} : new int {0}} {}
    ~What() noexcept { CERR("What({}) dies...\n", *p); delete p; }
};

struct Move
{
    int* p {};

    Move() : p {new int {0}} {}
    Move(int i) : p {new int {i}} {}
    Move(const Move& r) : p {r.p ? new int {*r.p} : new int {0}} {}
    Move(Move&& r) noexcept : p {std::exchange(r.p, new int {0})} { CERR("Move({}) moves...\n", *p); }
    ~Move() noexcept { CERR("Move({}) dies...\n", *p); delete p; }

    bool operator<(const Move& r) const { return *p < *r.p; }
    bool operator<=(const Move& r) const { return *p <= *r.p; }
    bool operator>(const Move& r) const { return *p > *r.p; }
    bool operator>=(const Move& r) const { return *p >= *r.p; }
    bool operator==(const Move& r) const { return *p == *r.p; }
    bool operator!=(const Move& r) const { return *p != *r.p; }
};

struct Virtual
{
    virtual ~Virtual() noexcept {};
};

struct V0 : Virtual
{
    int* p;

    V0() : p {} {}
    V0(int i) : p {new int {i}} {}
    V0(V0&& r) : p {std::exchange(r.p, nullptr)} {}
    ~V0() noexcept override { delete p; }
};

namespace adt::print
{

inline isize
format(Context ctx, FormatArgs fmtArgs, const What& x) noexcept
{
    return format(ctx, fmtArgs, *x.p);
}

inline isize
format(Context ctx, FormatArgs fmtArgs, const Move& x) noexcept
{
    return format(ctx, fmtArgs, *x.p);
}

inline isize
format(Context ctx, FormatArgs fmtArgs, const V0& x) noexcept
{
    if (x.p) return format(ctx, fmtArgs, *x.p);
    else return format(ctx, fmtArgs, "nullptr");
}

}
