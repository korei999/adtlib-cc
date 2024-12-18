#pragma once

#include "IAllocator.hh"
#include "hash.hh"

#include <cassert>
#include <cstring>
#include <immintrin.h>

namespace adt
{

[[nodiscard]] constexpr u32
nullTermStringSize(const char* nts)
{
    u32 i = 0;
    if (!nts) return 0;

    while (nts[i] != '\0') ++i;

    return i;
}

struct String;

[[nodiscard]] inline bool operator==(const String& l, const String& r);
[[nodiscard]] inline bool operator==(const String& l, const char* r);
[[nodiscard]] inline bool operator!=(const String& l, const String& r);
[[nodiscard]] constexpr s64 operator-(const String& l, const String& r);

/* StringAlloc() inserts '\0' char */
[[nodiscard]] inline String StringAlloc(IAllocator* p, const char* str, u32 size);
[[nodiscard]] inline String StringAlloc(IAllocator* p, u32 size);
[[nodiscard]] inline String StringAlloc(IAllocator* p, const char* str);
[[nodiscard]] inline String StringAlloc(IAllocator* p, const String s);

[[nodiscard]] inline String StringCat(IAllocator* p, const String l, const String r);

[[nodiscard]] inline u32 nGlyphs(const String str);

/* just pointer + size, no allocations */
struct String
{
    char* pData {};
    u32 size {};

    constexpr String() = default;
    constexpr String(char* sNullTerminated) : pData(sNullTerminated), size(nullTermStringSize(sNullTerminated)) {}
    constexpr String(const char* sNullTerminated) : pData(const_cast<char*>(sNullTerminated)), size(nullTermStringSize(sNullTerminated)) {}
    constexpr String(char* pStr, u32 len) : pData(pStr), size(len) {}

    constexpr char& operator[](u32 i)             { return pData[i]; }
    constexpr const char& operator[](u32 i) const { return pData[i]; }

    [[nodiscard]] constexpr bool endsWith(const String r) const;

    [[nodiscard]] constexpr u32 lastOf(char c) const;

    void destroy(IAllocator* p);

    void trimEnd();

    constexpr void removeNLEnd(); /* remove \r\n */

    [[nodiscard]] bool contains(const String r) const;

    struct It
    {
        char* p;

        constexpr It(char* pFirst) : p{pFirst} {}

        constexpr char& operator*() { return *p; }
        constexpr char* operator->() { return p; }

        constexpr It operator++() { p++; return *this; }
        constexpr It operator++(int) { char* tmp = p++; return tmp; }
        constexpr It operator--() { p--; return *this; }
        constexpr It operator--(int) { char* tmp = p--; return tmp; }

        friend constexpr bool operator==(It l, It r) { return l.p == r.p; }
        friend constexpr bool operator!=(It l, It r) { return l.p != r.p; }
    };

    constexpr It begin() { return {&this->pData[0]}; }
    constexpr It end() { return {&this->pData[this->size]}; }
    constexpr It rbegin() { return {&this->pData[this->size - 1]}; }
    constexpr It rend() { return {this->pData - 1}; }

    constexpr const It begin() const { return {&this->pData[0]}; }
    constexpr const It end() const { return {&this->pData[this->size]}; }
    constexpr const It rbegin() const { return {&this->pData[this->size - 1]}; }
    constexpr const It rend() const { return {this->pData - 1}; }
};

constexpr bool
StringEndsWith(String l, String r)
{
    if (l.size < r.size)
        return false;

    for (int i = r.size - 1, j = l.size - 1; i >= 0; --i, --j)
        if (r[i] != l[j])
            return false;

    return true;
}

constexpr bool
StringCmpSlow(const String l, const String r)
{
    if (l.size != r.size) return false;

    for (u32 i = 0; i < l.size; ++i)
        if (l[i] != r[i]) return false;

    return true;
}

ADT_NO_UB inline bool
StringCmpFast(const String& l, const String& r)
{
    if (l.size != r.size) return false;

    const u64* p0 = (u64*)l.pData;
    const u64* p1 = (u64*)r.pData;
    u32 len = l.size / 8;

    u32 i = 0;
    for (; i < len; ++i)
        if (p0[i] - p1[i] != 0) return false;

    if (l.size > 8)
    {
        const u64* t0 = (u64*)&l.pData[l.size - 9];
        const u64* t1 = (u64*)&r.pData[l.size - 9];
        return *t0 == *t1;
    }

    u32 leftOver = l.size - i*8;
    String nl(&l.pData[i*8], leftOver);
    String nr(&r.pData[i*8], leftOver);

    return StringCmpSlow(nl, nr);
}

#ifdef ADT_SSE4_2
inline bool
StringCmpSSE(const String& l, const String& r)
{
    if (l.size != r.size) return false;

    const __m128i* p0 = (__m128i*)l.pData;
    const __m128i* p1 = (__m128i*)r.pData;
    u32 len = l.size / 16;

    u32 i = 0;
    for (; i < len; ++i)
    {
        auto res = _mm_xor_si128(_mm_loadu_si128(&p0[i]), _mm_loadu_si128(&p1[i]));
        if (_mm_testz_si128(res, res) != 1) return false;
    }

    if (l.size > 16)
    {
        auto lv = _mm_loadu_si128((__m128i*)&l.pData[l.size - 17]);
        auto rv = _mm_loadu_si128((__m128i*)&r.pData[l.size - 17]);
        auto res = _mm_xor_si128(lv, rv);
        return _mm_testz_si128(res, res) == 1;
    }

    u32 leftOver = l.size - i*16;
    String nl(&l.pData[i*16], leftOver);
    String nr(&r.pData[i*16], leftOver);

    return StringCmpFast(nl, nr);
}
#endif

#ifdef ADT_AVX2
constexpr bool
StringCmpAVX2(const String& l, const String& r)
{
    if (l.size != r.size) return false;

    const __m256i* p0 = (__m256i*)l.pData;
    const __m256i* p1 = (__m256i*)r.pData;
    u32 len = l.size / 32;

    u32 i = 0;
    for (; i < len; ++i)
    {
        __m256i res = _mm256_xor_si256(_mm256_loadu_si256(&p0[i]), _mm256_loadu_si256(&p1[i]));
        if (_mm256_testz_si256(res, res) != 1) return false;
    }

    if (l.size > 32)
    {
        auto lv = _mm256_loadu_si256((__m256i*)&l.pData[l.size - 33]);
        auto rv = _mm256_loadu_si256((__m256i*)&r.pData[l.size - 33]);
        __m256i res = _mm256_xor_si256(lv, rv);
        return _mm256_testz_si256(res, res) == 1;
    }

    u32 leftOver = l.size - i*32;
    String nl(&l.pData[i*32], leftOver);
    String nr(&r.pData[i*32], leftOver);

    return StringCmpFast(nl, nr);
}
#endif

inline bool
operator==(const String& l, const String& r)
{
#if defined(ADT_SSE4_2) && !defined(ADT_AVX2)
    return StringCmpSSE(l, r);
#endif

#ifdef ADT_AVX2
    if (l.size >= 32)
        return StringCmpAVX2(l, r);
    else if (l.size >= 16)
        return StringCmpSSE(l, r);
    else return StringCmpFast(l, r);
#endif

#if !defined(ADT_SSE4_2) && !defined(ADT_AVX2)
    if (l.size != r.size) return false;
    return strncmp(l.pData, r.pData, l.size) == 0;
#endif
}

inline bool
operator==(const String& l, const char* r)
{
    auto sr = String(r);
    return l == sr;
}

inline bool
operator!=(const String& l, const String& r)
{
    return !(l == r);
}

constexpr s64
operator-(const String& l, const String& r)
{
    if (l.size < r.size) return -1;
    else if (l.size > r.size) return 1;

    s64 sum = 0;
    for (u32 i = 0; i < l.size; i++)
        sum += (l[i] - r[i]);

    return sum;
}

constexpr u32
StringLastOf(String sv, char c)
{
    for (int i = sv.size - 1; i >= 0; i--)
        if (sv[i] == c)
            return i;

    return NPOS;
}

inline String
StringAlloc(IAllocator* p, const char* str, u32 size)
{
    if (str == nullptr || size == 0) return {};

    char* pData = (char*)p->zalloc(size + 1, sizeof(char));
    strncpy(pData, str, size);
    pData[size] = '\0';

    return {pData, size};
}

inline String
StringAlloc(IAllocator* p, u32 size)
{
    if (size == 0) return {};

    char* pData = (char*)p->zalloc(size + 1, sizeof(char));

    return {pData, size};
}

inline String
StringAlloc(IAllocator* p, const char* str)
{
    return StringAlloc(p, str, nullTermStringSize(str));
}

inline String
StringAlloc(IAllocator* p, const String s)
{
    return StringAlloc(p, s.pData, s.size);
}

inline void
String::destroy(IAllocator* p)
{
    p->free(this->pData);
    *this = {};
}

inline String
StringCat(IAllocator* p, const String l, const String r)
{
    u32 len = l.size + r.size;
    char* ret = (char*)p->zalloc(len + 1, sizeof(char));

    u32 pos = 0;
    for (u32 i = 0; i < l.size; ++i, ++pos)
        ret[pos] = l[i];
    for (u32 i = 0; i < r.size; ++i, ++pos)
        ret[pos] = r[i];

    ret[len] = '\0';

    return {ret, len};
}

inline void
String::trimEnd()
{
    auto isWhiteSpace = [&](int i) -> bool {
        char c = this->pData[i];
        if (c == '\n' || c == ' ' || c == '\r' || c == '\t' || c == '\0')
            return true;

        return false;
    };

    for (int i = this->size - 1; i >= 0; --i)
        if (isWhiteSpace(i))
        {
            this->pData[i] = 0;
            --this->size;
        }
        else break;
}

constexpr void
String::removeNLEnd()
{
    auto oneOf = [&](char c) -> bool {
        constexpr String chars = "\r\n";
        for (auto ch : chars)
            if (c == ch) return true;
        return false;
    };

    u64 pos = this->size - 1;
    while (this->size > 0 && oneOf((*this)[pos]))
        this->pData[--this->size] = '\0';
}

inline bool
String::contains(const String r) const
{
    if (this->size < r.size || this->size == 0 || r.size == 0) return false;

    for (u32 i = 0; i < this->size - r.size + 1; ++i)
    {
        const String sSub {const_cast<char*>(&(*this)[i]), r.size};
        if (sSub == r) return true;
    }

    return false;
}

inline u32
nGlyphs(const String str)
{
    u32 n = 0;
    for (u32 i = 0; i < str.size; )
    {
        i+= mblen(&str[i], str.size - i);
        ++n;
    }

    return n;
}

template<>
constexpr u64
hash::func(String& str)
{
    return hash::fnvStr(str.pData, str.size);
}

template<>
constexpr u64
hash::func(const String& str)
{
    return hash::fnvStr(str.pData, str.size);
}

template<>
inline u64
hash::funcHVal(String& str, u64 hashValue)
{
    return hash::fnvBuffHVal(str.pData, str.size, hashValue);
}

template<>
inline u64
hash::funcHVal(const String& str, u64 hashValue)
{
    return hash::fnvBuffHVal(str.pData, str.size, hashValue);
}

namespace utils
{

[[nodiscard]] constexpr bool
empty(const String* s)
{
    return s->size == 0;
}

} /* namespace utils */

} /* namespace adt */
