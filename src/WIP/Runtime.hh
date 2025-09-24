#pragma once

#include "adt/ILogger.hh"
#include "adt/IThreadPool.hh"

namespace adt
{

struct Runtime
{
    ILogger* pLogger {};
    IThreadPool* pThreadPool {};
};

} /* namespace adt */

