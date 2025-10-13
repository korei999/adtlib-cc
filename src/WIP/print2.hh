#pragma once

#include "print2-inl.hh"

#include "adt/assert.hh"
#include "adt/Array.hh"

#include <cctype>
#include <charconv>

namespace adt::print2
{

inline
Builder::Builder(IAllocator* pAlloc, isize prealloc)
    : m_pAlloc{pAlloc},
      m_pData{pAlloc->mallocV<char>(prealloc + 1)},
      m_cap{prealloc + 1},
      m_bAllocated{true}
{
}

inline
Builder::Builder(Span<char> spBuff)
    : m_pData{spBuff.data()},
      m_cap{spBuff.size()}
{
}

inline void
Builder::destroy() noexcept
{
    if (m_bAllocated) m_pAlloc->free(m_pData, m_cap);
    *this = {};
}

inline isize
Builder::push(char c)
{
    if (m_size + 1 >= m_cap)
        if (!growIfNeeded(m_cap + 1) * 2) return -1;

    m_pData[m_size++] = c;
    m_pData[m_size] = '\0';
    return 1;
}

inline isize
Builder::push(const StringView sv)
{
    if (sv.m_size + m_size >= m_cap)
    {
        if (!m_pAlloc)
        {
            const isize maxPossbile = m_cap - 1 - m_size;
            if (maxPossbile <= 0) return -1;
            ::memcpy(m_pData + m_size, sv.m_pData, maxPossbile);
            m_size += maxPossbile;
            m_pData[m_cap - 1] = '\0';
            return maxPossbile;
        }

        if (!growIfNeeded((m_size + sv.m_size + 1) * 2)) return -1;
    }

    ::memcpy(m_pData + m_size, sv.m_pData, sv.m_size);
    m_size += sv.m_size;
    m_pData[m_size] = '\0';
    return sv.m_size;
}

inline isize
Builder::push(StringView sv, FmtArgs* pFmtArgs)
{
    if (sv.m_size <= 0) return 0;

    const isize maxSvLen = pFmtArgs->maxLen > 0 ? utils::min(pFmtArgs->maxLen, sv.m_size) : sv.m_size;
    isize padSize = utils::max(pFmtArgs->padding, pFmtArgs->maxLen);
    if (padSize > 0) padSize -= maxSvLen;
    else padSize = 0;

    isize nWritten = 0;

    sv = sv.subString(0, maxSvLen);

    if (sv.m_size + m_size + padSize >= m_cap)
    {
        if (!m_pAlloc)
        {
            isize maxPossbile = m_cap - 1 - m_size;
            if (maxPossbile <= 0) goto done;

            if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_RIGHT))
            {
                if (padSize > 0)
                {
                    isize maxPad = utils::min(maxPossbile, padSize);
                    ::memset(m_pData + m_size, pFmtArgs->filler, maxPad);
                    m_size += maxPad;
                    nWritten += maxPad;
                    maxPossbile -= maxPad;
                }
                if (maxPossbile > 0)
                {
                    isize maxSv = utils::min(maxPossbile, sv.m_size);
                    ::memcpy(m_pData + m_size, sv.m_pData, maxSv);
                    m_size += maxSv;
                    nWritten += maxSv;
                }
            }
            else
            {
                isize maxSv = utils::min(maxPossbile, sv.m_size);
                ::memcpy(m_pData + m_size, sv.m_pData, maxSv);
                m_size += maxSv;
                nWritten += maxSv;
                maxPossbile -= maxSv;
                if (maxPossbile > 0)
                {
                    isize maxPad = utils::min(maxPossbile, padSize);
                    ::memset(m_pData + m_size, pFmtArgs->filler, maxPad);
                    m_size += maxPad;
                    nWritten += maxPad;
                }
            }

            goto done;
        }

        if (!growIfNeeded((sv.m_size + m_size + padSize + 1) * 2)) return -1;
    }

    if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_RIGHT))
    {
        if (padSize > 0)
        {
            ::memset(m_pData + m_size, pFmtArgs->filler, padSize);
            m_size += padSize;
            nWritten += padSize;
        }
        ::memcpy(m_pData + m_size, sv.m_pData, sv.m_size);
        m_size += sv.m_size;
        nWritten += sv.m_size;
    }
    else
    {
        ::memcpy(m_pData + m_size, sv.m_pData, sv.m_size);
        m_size += sv.m_size;
        nWritten += sv.m_size;
        if (padSize > 0)
        {
            ::memset(m_pData + m_size, pFmtArgs->filler, padSize);
            m_size += padSize;
            nWritten += padSize;
        }
    }

done:
    m_pData[m_size] = '\0';
    return nWritten;
}

inline bool
Builder::growIfNeeded(isize newCap)
{
    if (!m_pAlloc) return false;

    try
    {
        if (!m_bAllocated)
        {
            char* pNewData = m_pAlloc->mallocV<char>(newCap);
            ::memcpy(pNewData, m_pData, m_size);
            m_pData = pNewData;
            m_cap = newCap;
        }
        else
        {
            m_pData = m_pAlloc->reallocV<char>(m_pData, m_size, newCap);
            m_cap = newCap;
        }
    }
    catch (const std::bad_alloc& ex)
    {
        return false;
    }

    return true;
}

template<typename T>
constexpr const StringView typeName()
{
#ifdef __clang__
    return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
    return "unsupported compiler";
#endif
}

template<typename T>
inline isize
format(Context* pCtx, FmtArgs* pFmtArgs, const T& arg)
{
    const StringView sv = typeName<T>();

#if defined __clang__ || __GNUC__

    const StringView svSub = "T = ";
    const isize atI = sv.subStringAt(svSub);
    const StringView svDemangled = [&]
    {
        if (atI != NPOS)
        {
            return StringView {
                const_cast<char*>(sv.data() + atI + svSub.size()),
                    sv.size() - atI - svSub.size() - 1
            };
        }
        else
        {
            return sv;
        }
    }();

#elif defined _WIN32

    const StringView svSub = "typeName<";
    const isize atI = sv.subStringAt(svSub);
    const StringView svDemangled = [&]
    {
        if (atI != NPOS)
        {
            return StringView {
                const_cast<char*>(sv.data() + atI + svSub.size()),
                    sv.size() - atI - svSub.size() - 1
            };
        }
        else
        {
            return sv;
        }
    }();

#else

    const StringView svDemangled = sv;

#endif

    return pCtx->pBuilder->push(svDemangled);
}

namespace details
{

template<typename T>
inline TypeErasedArg createTypeErasedArg(const T& arg);

inline isize parseNumber(Context* pCtx, FmtArgs* pFmtArgs);

inline isize parseChar(Context* pCtx, FmtArgs* pFmtArgs);

inline isize parseColon(Context* pCtx, FmtArgs* pFmtArgs);

inline isize parseArg(Context* pCtx, FmtArgs* pFmtArgs);

inline isize parseArgs(Context* pCtx, FmtArgs* pFmtArgs);

} /* namespace details */

namespace details
{

template<typename T>
inline TypeErasedArg
createTypeErasedArg(const T& arg)
{
    TypeErasedArg::PfnFormat pfn = [](Context* pCtx, FmtArgs* pFmtArgs, const void* pArg) {
        const T& r = *static_cast<const T*>(pArg);

        if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::COLON))
        {
            if constexpr (std::is_integral_v<T>)
            {
                if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::FLOAT_PRECISION))
                {
                    pFmtArgs->eFlags &= ~FmtArgs::FLAGS::FLOAT_PRECISION;
                    pFmtArgs->floatPrecision = r;
                }
                else if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::FILLER))
                {
                    pFmtArgs->eFlags &= ~FmtArgs::FLAGS::FILLER;
                    pFmtArgs->filler = r;
                }
                else if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_LEFT))
                {
                    pFmtArgs->padding = r;
                }
                else if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_RIGHT))
                {
                    pFmtArgs->padding = r;
                }
                else
                {
                    pFmtArgs->maxLen = r;
                }

                return FMT_ARG_SET;
            }
        }

        return format(pCtx, pFmtArgs, r);;
    };

    return TypeErasedArg{
        .pfnFormat = pfn,
        .pArg = &arg,
    };
}

inline isize
parseNumber(Context* pCtx, FmtArgs* pFmtArgs)
{
    isize nWritten = 0;
    isize startI = pCtx->fmtI;

    if (pCtx->fmtI < pCtx->svFmt.m_size && pCtx->svFmt[pCtx->fmtI] == '{')
    {
        ++pCtx->fmtI;
        nWritten += parseArg(pCtx, pFmtArgs);
    }
    else
    {
        while (pCtx->fmtI < pCtx->svFmt.m_size && std::isdigit(pCtx->svFmt[pCtx->fmtI]))
            ++pCtx->fmtI;

        const StringView svNum = pCtx->svFmt.subString(startI, pCtx->fmtI - startI);

        if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_LEFT))
        {
            pFmtArgs->padding = svNum.toI64();
        }
        else if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::JUSTIFY_RIGHT))
        {
            pFmtArgs->padding = svNum.toI64();
        }
        else if (bool(pFmtArgs->eFlags & FmtArgs::FLAGS::FLOAT_PRECISION))
        {
            pFmtArgs->eFlags &= ~FmtArgs::FLAGS::FLOAT_PRECISION;
            pFmtArgs->floatPrecision = svNum.toI64();
        }
        else
        {
            pFmtArgs->maxLen = svNum.toI64();
        }
    }

    return nWritten;
}

inline isize
parseChar(Context* pCtx, FmtArgs* pFmtArgs)
{
    isize nWritten = 0;

    if (pCtx->fmtI >= pCtx->svFmt.m_size) return 0;

    if (pCtx->svFmt[pCtx->fmtI] == '{')
    {
        pFmtArgs->eFlags |= FmtArgs::FLAGS::FILLER;
        ++pCtx->fmtI;
        nWritten += parseArg(pCtx, pFmtArgs);
    }
    else
    {
        pFmtArgs->filler = pCtx->svFmt[pCtx->fmtI];
    }

    return nWritten;
}

inline isize
parseColon(Context* pCtx, FmtArgs* pFmtArgs)
{
    while (pCtx->fmtI < pCtx->svFmt.m_size && pCtx->svFmt[pCtx->fmtI] != '}')
    {
        if (pCtx->svFmt[pCtx->fmtI] == '{')
        {
            parseArg(pCtx, pFmtArgs);
            continue;
        }
        else if (std::isdigit(pCtx->svFmt[pCtx->fmtI]))
        {
            parseNumber(pCtx, pFmtArgs);
            continue;
        }
        else if (pCtx->svFmt[pCtx->fmtI] == 'f')
        {
            ++pCtx->fmtI;
            parseChar(pCtx, pFmtArgs);
            continue;
        }
        else if (pCtx->svFmt[pCtx->fmtI] == '<')
        {
            ++pCtx->fmtI;
            pFmtArgs->eFlags |= FmtArgs::FLAGS::JUSTIFY_LEFT;
            parseNumber(pCtx, pFmtArgs);
            continue;
        }
        else if (pCtx->svFmt[pCtx->fmtI] == '>')
        {
            ++pCtx->fmtI;
            pFmtArgs->eFlags |= FmtArgs::FLAGS::JUSTIFY_RIGHT;
            parseNumber(pCtx, pFmtArgs);
            continue;
        }
        else if (pCtx->svFmt[pCtx->fmtI] == '.')
        {
            ++pCtx->fmtI;
            pFmtArgs->eFlags |= FmtArgs::FLAGS::FLOAT_PRECISION;
            parseNumber(pCtx, pFmtArgs);
            continue;
        }

        ++pCtx->fmtI;
    }

    return 0;
}

inline isize
parseArg(Context* pCtx, FmtArgs* pFmtArgs)
{
    isize nWritten = 0;

    while (pCtx->fmtI < pCtx->svFmt.m_size && pCtx->svFmt[pCtx->fmtI] != '}')
    {
        if (pCtx->svFmt[pCtx->fmtI] == ':')
        {
            pFmtArgs->eFlags |= FmtArgs::FLAGS::COLON;
            ++pCtx->fmtI;
            isize n = parseColon(pCtx, pFmtArgs);
            if (n > 0) nWritten += n;
            continue;
        }

        ++pCtx->fmtI;
    }

    if (pCtx->argI >= 0 && pCtx->argI < pCtx->spArgs.m_size)
    {
        auto arg = pCtx->spArgs[pCtx->argI++];
        isize n = arg.pfnFormat(pCtx, pFmtArgs, arg.pArg);
        if (n != FMT_ARG_SET && n > 0) nWritten += n;
    }
    else
    {
        isize n = pCtx->pBuilder->push("(missing arg)");
        if (n > 0) nWritten += n;
    }

    if (pCtx->fmtI < pCtx->svFmt.m_size) ++pCtx->fmtI;

    return nWritten;
}

inline isize
parseArgs(Context* pCtx, FmtArgs* pFmtArgs)
{
    isize nWritten = 0;

    while (pCtx->fmtI < pCtx->svFmt.m_size)
    {
        StringView svCurr = pCtx->svFmt.subString(pCtx->fmtI);
        const isize nextParenOff = svCurr.charAt('{');
        if (nextParenOff == -1) break;

        {
            const isize n = pCtx->pBuilder->push(svCurr.subString(0, nextParenOff));
            if (n <= -1) return nWritten;
            nWritten += n;
        }

        pCtx->fmtI += nextParenOff;
        ++pCtx->fmtI;
        if (pCtx->fmtI < pCtx->svFmt.m_size && pCtx->svFmt[pCtx->fmtI] == '{') /* Skip arg on double {{. */
        {
            pCtx->pBuilder->push(pCtx->svFmt[pCtx->fmtI]);
            ++pCtx->fmtI;
        }
        else
        {
            FmtArgs fmtArgs2 = *pFmtArgs;
            nWritten += parseArg(pCtx, &fmtArgs2);
        }
    }

    if (pCtx->fmtI < pCtx->svFmt.m_size)
    {
        StringView svCurr = pCtx->svFmt.subString(pCtx->fmtI);
        isize n = pCtx->pBuilder->push(svCurr);
        if (n > 0) nWritten += n;
    }

    return nWritten;
}

} /* namespace details */;

template<std::integral T>
inline isize
format(Context* pCtx, FmtArgs* pFmtArgs, const T& arg)
{
    char aBuff[128];
    isize nWritten = 0;

    static const char s_aCharSet[] = "0123456789abcdef";

#define _ITOA_(x, base)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        aBuff[nWritten++] = s_aCharSet[x % base];                                                                      \
        x /= base;                                                                                                     \
    } while (x > 0)

    T num = arg;
    if constexpr (!std::is_unsigned_v<T>)
        if (arg < 0) num = -num;

    _ITOA_(num, 10);
#undef _ITOA_

    if constexpr (!std::is_unsigned_v<T>)
        if (arg < 0) aBuff[nWritten++] = '-';

    for (isize i = 0; i < nWritten >> 1; ++i)
        utils::swap(&aBuff[i], &aBuff[nWritten - 1 - i]);

    const isize n = pCtx->pBuilder->push({aBuff, nWritten});
    if (n <= -1) return 0;
    return n;
}

template<std::floating_point T>
inline isize
format(Context* pCtx, FmtArgs* pFmtArgs, const T& arg)
{
    char aBuff[128] {};
    std::to_chars_result res {};

    if (pFmtArgs->floatPrecision == -1)
        res = res = std::to_chars(aBuff, aBuff + sizeof(aBuff), arg);
    else res = std::to_chars(aBuff, aBuff + sizeof(aBuff), arg, std::chars_format::fixed, pFmtArgs->floatPrecision);

    isize n = 0;
    if (res.ptr) n = pCtx->pBuilder->push({aBuff, res.ptr - aBuff}, pFmtArgs);
    else n = pCtx->pBuilder->push({aBuff}, pFmtArgs);

    if (n <= -1) return 0;
    return n;
}

template<isize N>
inline isize
format(Context* pCtx, FmtArgs* pFmtArgs, const char(&arg)[N])
{
    return format(pCtx, pFmtArgs, StringView{arg});
}

template<>
inline isize
format(Context* pCtx, FmtArgs* pFmtArgs, const StringView& sv)
{
    const isize n = pCtx->pBuilder->push(sv, pFmtArgs);
    if (n <= -1) return 0;
    return n;
}

template<typename ...ARGS>
inline isize
toSpan(Span<char> spBuff, const StringView svFmt, const ARGS&... args)
{
    Array<TypeErasedArg, sizeof...(ARGS)> aArgs = {details::createTypeErasedArg(args)...};

    Builder builder {spBuff};
    Context ctx {.spArgs = aArgs, .svFmt = svFmt, .pBuilder = &builder};
    FmtArgs fmtArgs {};
    return details::parseArgs(&ctx, &fmtArgs);
}

template<typename ...ARGS>
inline isize
toBuffer(char* pBuff, isize buffSize, const StringView svFmt, const ARGS&... args)
{
    Array<TypeErasedArg, sizeof...(ARGS)> aArgs = {details::createTypeErasedArg(args)...};

    Builder builder {{pBuff, buffSize}};
    Context ctx {.spArgs = aArgs, .svFmt = svFmt, .pBuilder = &builder};
    FmtArgs fmtArgs {};
    return details::parseArgs(&ctx, &fmtArgs);
}

} /* namespace adt::print2 */
