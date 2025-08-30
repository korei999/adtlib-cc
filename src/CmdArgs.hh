#pragma once

#include "adt/Vec.hh"

namespace adt
{

struct CmdArgs
{
    template<typename STRING>
    struct ArgParser
    {
        bool bNeedsValue {};
        StringFixed<3> sfOneDash {}; /* May be empty. */
        STRING sTwoDashes {}; /* May be empty (but not both). */
        STRING sDescription {};
        void (*pfnParse)(
            void* pAny,
            const StringView svKey,
            const StringView svVal /* Empty if !bNeedsValue.*/
        ) noexcept {};
    };

    /* */

    IAllocator* m_pAlloc {};
    Vec<ArgParser<String>> m_vArgParsers {};
    int m_argc {};
    char** m_argv {};

    /* */

    CmdArgs(IAllocator* pAlloc, int argc, char** argv, std::initializer_list<ArgParser<StringView>> lParsers);

    /* */

    void destroy() noexcept;
};

inline
CmdArgs::CmdArgs(IAllocator* pAlloc, int argc, char** argv, std::initializer_list<ArgParser<StringView>> lParsers)
    : m_pAlloc{pAlloc}, m_argc{argc}, m_argv{argv}
{
    for (auto& e : lParsers)
    {
        m_vArgParsers.push(m_pAlloc, {
            .bNeedsValue = e.bNeedsValue,
            .sfOneDash = e.sfOneDash,
            .sTwoDashes = {m_pAlloc, e.sTwoDashes},
            .sDescription = {m_pAlloc, e.sDescription},
            .pfnParse = e.pfnParse
        });
    }
}

inline void
CmdArgs::destroy() noexcept
{
    for (auto& e : m_vArgParsers)
    {
        e.sTwoDashes.destroy(m_pAlloc);
        e.sDescription.destroy(m_pAlloc);
    }
    m_vArgParsers.destroy(m_pAlloc);
}

} /* namespace adt */
