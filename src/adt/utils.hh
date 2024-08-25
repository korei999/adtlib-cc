#pragma once

#ifdef __linux__
    #include <time.h>
#elif _WIN32
    #include <sysinfoapi.h>
    #include <windows.h>

    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif
    #ifdef near
        #undef near
    #endif
    #ifdef far
        #undef far
    #endif
#endif

#include "types.hh"

namespace adt
{
namespace utils
{

template<typename A, typename B> constexpr A& max(A& l, B& r);
template<typename A, typename B> constexpr A& min(A& l, B& r);
template<typename T> constexpr u64 size(const T& a);
template<typename T> constexpr bool odd(const T& a);
template<typename T> constexpr bool even(const T& a);
template<typename T> constexpr s64 compare(const T& l, const T& r);
inline f64 timeNowMS();
inline f64 timeNowS();

template<typename A, typename B>
constexpr A&
max(A& l, B& r)
{
    return l > r ? l : r;
}

template<typename A, typename B>
constexpr A&
min(A& l, B& r)
{
    return l < r ? l : r;
}

template<typename T>
constexpr u64
size(const T& a)
{
    return sizeof(a) / sizeof(a[0]);
}

template<typename T>
constexpr bool
odd(const T& a)
{
    return a & 1;
}

template<typename T>
constexpr bool
even(const T& a)
{
    return !odd(a);
}

/* negative is l < r, positive if l > r, 0 if l == r */
template<typename T>
constexpr s64
compare(const T& l, const T& r)
{
    return l - r;
}

inline f64
timeNowMS()
{
#ifdef __linux__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_t micros = ts.tv_sec * 1000000000;
    micros += ts.tv_nsec;

    return micros / 1000000.0;

#elif _WIN32
    LARGE_INTEGER count, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);

    auto ret = (count.QuadPart * 1000000) / freq.QuadPart;
    return f64(ret) / 1000.0;
#endif
}

inline f64
timeNowS()
{
    return timeNowMS() / 1000.0;
}

} /* namespace utils */
} /* namespace adt */
