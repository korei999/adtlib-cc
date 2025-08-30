#pragma once

#include "adt/Map.hh"
#include "adt/Vec.hh"
#include "adt/Logger.hh"

namespace adt
{

struct ArgvParser
{
    template<typename STRING>
    struct ArgParser
    {
        bool bNeedsValue {};
        STRING sOneDash {}; /* May be empty. */
        STRING sTwoDashes {}; /* May be empty (but not both). */
        STRING sDescription {};
        void (*pfn)(
            void* pAny,
            const StringView svKey,
            const StringView svVal /* Empty if !bNeedsValue.*/
        ) noexcept {};
        void* pAnyData {};
    };

    /* */

    IAllocator* m_pAlloc {};
    Vec<ArgParser<String>> m_vArgParsers {};
    Map<StringView, isize> m_mStringToArgI {};
    int m_argc {};
    char** m_argv {};

    /* */

    ArgvParser() noexcept = default;
    ArgvParser(IAllocator* pAlloc, int argc, char** argv, std::initializer_list<ArgParser<StringView>> lParsers);

    /* */

    void destroy() noexcept;
    bool parse() noexcept;
};

inline
ArgvParser::ArgvParser(IAllocator* pAlloc, int argc, char** argv, std::initializer_list<ArgParser<StringView>> lParsers)
    : m_pAlloc{pAlloc}, m_vArgParsers{pAlloc, argc}, m_mStringToArgI{pAlloc, argc}, m_argc{argc}, m_argv{argv}
{

    for (auto& e : lParsers)
    {
        ADT_ASSERT(!e.sOneDash.empty() || !e.sTwoDashes.empty(),
            "oneDash: '{}', sTwoDashes: '{}'",
            e.sOneDash, e.sTwoDashes
        );

        const isize idx = m_vArgParsers.push(m_pAlloc, {
            .bNeedsValue = e.bNeedsValue,
            .sOneDash = {m_pAlloc, e.sOneDash},
            .sTwoDashes = {m_pAlloc, e.sTwoDashes},
            .sDescription = {m_pAlloc, e.sDescription},
            .pfn = e.pfn
        });

        if (!e.sOneDash.empty())
            m_mStringToArgI.insert(m_pAlloc, e.sOneDash, idx);
        if (!e.sTwoDashes.empty())
            m_mStringToArgI.insert(m_pAlloc, m_vArgParsers[idx].sTwoDashes, idx);
    }
}

inline void
ArgvParser::destroy() noexcept
{
    for (auto& e : m_vArgParsers)
    {
        e.sOneDash.destroy(m_pAlloc);
        e.sTwoDashes.destroy(m_pAlloc);
        e.sDescription.destroy(m_pAlloc);
    }
    m_vArgParsers.destroy(m_pAlloc);
    m_mStringToArgI.destroy(m_pAlloc);
}

inline bool
ArgvParser::parse() noexcept
{
    for (isize i = 1; i < m_argc; ++i)
    {
        const StringView svArg = m_argv[i];

        if (svArg.beginsWith("--"))
        {
            const StringView svTwoDash = svArg.subString(2, svArg.size() - 2);
        }
        else if (svArg.beginsWith("-"))
        {
            const StringView svKey = svArg.subString(1, svArg.size() - 1);
            MapResult res = m_mStringToArgI.search(svKey);
            if (res)
            {
                const isize idx = res.value();
                auto& rParser = m_vArgParsers[idx];

                if (rParser.bNeedsValue)
                {
                    if (i + 1 >= m_argc)
                    {
                        print::out("argument '{}' expects value\n", svKey);
                        return false;
                    }

                    const StringView svVal = m_argv[i + 1];
                    rParser.pfn(rParser.pAnyData, svKey, svVal);
                }
                else
                {
                    rParser.pfn(rParser.pAnyData, svKey, {});
                }
            }
        }
    }

    return true;
}

} /* namespace adt */
