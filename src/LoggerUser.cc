#ifdef PLUGIN_SOURCE
#error "must be defined by the plugin source"
#endif

#define PLUGIN_SOURCE

#include "LoggerUser.hh"

#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"

using namespace adt;

PLUGIN_API void
pluginInit(adt::IThreadPool* pTp, adt::ILogger* pLogger) noexcept
{
    adt::IThreadPool::setGlobal(pTp);
    adt::ILogger::setGlobal(pLogger);
}

PLUGIN_API void
pluginLoggingFunc() noexcept
{
    LogWarn("hello from 'LoggerUser'\n");
}

PLUGIN_API void
pluginThreadLocalThing() noexcept
{
    LogInfo{"hello from threadId: {}\n", IThreadPool::inst()->threadId()};
}

#undef PLUGIN_SOURCE
