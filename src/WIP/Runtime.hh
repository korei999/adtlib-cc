#pragma once

#include "adt/ILogger.hh"
#include "adt/IThreadPool.hh"

namespace adt
{

struct Context
{
    static inline Context* g_pContext {};

    static Context* inst() noexcept { return g_pContext; }

    /* */

    ILogger* m_pLogger {};
    IThreadPool* m_pThreadPool {}; /* Owns thread_local arenas. */
};

} /* namespace adt */

