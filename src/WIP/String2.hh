#pragma once

#include "adt/IAllocator.hh"
#include "adt/assert.hh"

namespace adt
{

/* Small string optimized String class. */
struct String2
{
    union {
        StringFixed<16> m_sf {};
        struct {
            char* m_pData;
            isize m_size;
        };
    };
    isize m_cap = 16;

    /* */

    String2() = default;
    String2(IAllocator* pAlloc, const StringView sv);

    operator StringView() noexcept { return StringView(data(), size()); }
    operator const StringView() const noexcept { return StringView(const_cast<char*>(data()), size()); }

    /* */

    void destroy(IAllocator* pAlloc) noexcept;

    char* data() noexcept;
    const char* data() const noexcept;

    isize size() const noexcept;
    isize cap() const noexcept;

    isize push(IAllocator* pAlloc, char c);
    isize push(IAllocator* pAlloc, const StringView sv);

    void reallocWith(IAllocator* pAlloc, const StringView sv);
    void removeNLEnd(bool bDestructive) noexcept;

protected:
    void grow(IAllocator* pAlloc, isize newCap);
};

inline void
String2::destroy(IAllocator* pAlloc) noexcept
{
    if (m_cap >= 17) pAlloc->free(m_pData);
    m_cap = 16;
    m_sf.destroy();
}

inline char*
String2::data() noexcept
{
    if (m_cap <= 16) return m_sf.data();
    else return m_pData;
}

inline const char*
String2::data() const noexcept
{
    if (m_cap <= 16) return m_sf.data();
    else return m_pData;
}

inline isize
String2::size() const noexcept
{
    if (m_cap <= 16) return m_sf.size();
    else return m_size;
}

inline isize
String2::cap() const noexcept
{
    return m_cap;
}

inline
String2::String2(IAllocator* pAlloc, const StringView sv)
{
    if (sv.m_size <= 15)
    {
        ::memcpy(m_sf.data(), sv.m_pData, sv.m_size);
        m_sf.data()[sv.m_size] = '\0';
    }
    else
    {
        m_pData = pAlloc->mallocV<char>(sv.m_size + 1);
        ::memcpy(m_pData, sv.m_pData, sv.m_size);
        m_pData[sv.m_size] = '\0';
        m_cap = sv.m_size + 1;
        m_size = sv.m_size;
    }
}

inline isize
String2::push(IAllocator* pAlloc, char c)
{
    ADT_ASSERT(m_cap >= 16, "{}", m_cap);
    if (m_cap == 16)
    {
        const isize firstSize = m_sf.size();
        if (firstSize + 1 >= 16)
        {
            const isize newCap = m_cap * 2;
            char* pNew = pAlloc->zallocV<char>(newCap);
            ::memcpy(pNew, m_sf.data(), firstSize);
            pNew[firstSize] = c;

            m_pData = pNew;
            m_size = firstSize + 1;
            m_cap = newCap;

            return firstSize;
        }

        m_sf.data()[firstSize] = c;
        return firstSize;
    }
    else
    {
        ADT_ASSERT(m_cap > 16, "{}", m_cap);

        if (m_size >= m_cap) grow(pAlloc, m_cap * 2);

        m_pData[m_size++] = c;
        return m_size - 1;
    }
}

inline isize
String2::push(IAllocator* pAlloc, const StringView sv)
{
    ADT_ASSERT(m_cap >= 16, "{}", m_cap);
    if (m_cap == 16)
    {
        const isize firstSize = m_sf.size();
        if (sv.m_size + firstSize + 1 > 16)
        {
            const isize newCap = nextPowerOf2(sv.m_size + firstSize + 1);
            char* pNew = pAlloc->zallocV<char>(newCap);
            ::memcpy(pNew, m_sf.data(), firstSize);
            ::memcpy(pNew + firstSize, sv.m_pData, sv.m_size);

            m_pData = pNew;
            m_size = firstSize + sv.m_size;
            m_cap = newCap;

            return firstSize;
        }
        else
        {
            ::memcpy(m_sf.data() + firstSize, sv.m_pData, sv.m_size);
            return firstSize;
        }
    }
    else
    {
        ADT_ASSERT(m_cap > 16, "{}", m_cap);

        if (m_size + sv.m_size >= m_cap)
            grow(pAlloc, nextPowerOf2(m_cap + sv.m_size));

        ::memcpy(m_pData + m_size, sv.m_pData, sv.m_size);
        const isize ret = m_size;
        m_size += sv.m_size;
        return ret;
    }
}

inline void
String2::reallocWith(IAllocator* pAlloc, const StringView sv)
{
    ADT_ASSERT(m_cap >= 16, "{}", m_cap);
    if (m_cap <= 16)
    {
        const isize firstSize = m_sf.size();
        if (sv.m_size > 15)
        {
            const isize newCap = nextPowerOf2(sv.m_size + 1);
            char* pNew = pAlloc->zallocV<char>(newCap);
            ::memcpy(pNew, sv.m_pData, sv.m_size);
            m_pData = pNew;
            m_size = sv.m_size;
            m_cap = newCap;

            return;
        }

        ::memcpy(m_sf.data(), sv.m_pData, sv.m_size);
        m_sf.data()[sv.m_size] = '\0';
    }
    else
    {
        if (sv.m_size + 1 > m_cap) grow(pAlloc, nextPowerOf2(sv.m_size + 1));
        ::memcpy(m_pData, sv.m_pData, sv.m_size);
        m_pData[sv.m_size] = '\0';
        m_size = sv.m_size;
    }
}

inline void
String2::removeNLEnd(bool bDestructive) noexcept
{
    ADT_ASSERT(m_cap >= 16, "{}", m_cap);

    isize size;
    char* pData;

    if (m_cap <= 16)
    {
        size = m_sf.size();
        pData = m_sf.data();
    }
    else
    {
        pData = m_pData;
        size = m_size;
    }

    while (size > 0 && (pData[size - 1] == '\n' || pData[size - 1] == '\r'))
    {
        if (bDestructive) pData[size - 1] = '\0';
        --size;
    }

    if (m_cap > 16) m_size = size;
}

inline void
String2::grow(IAllocator* pAlloc, isize newCap)
{
    ADT_ASSERT(m_cap >= 17, "{}", m_cap);
    m_pData = pAlloc->reallocV<char>(m_pData, m_size, newCap);
    m_cap = newCap;
}

} /* namespace adt */
