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
pluginInit(adt::ILogger* p) noexcept
{
    adt::ILogger::setGlobal(p);
}

PLUGIN_API void
pluginLoggingFunc() noexcept
{
    LogWarn("hello from 'LoggerUser'\n");
}

PLUGIN_API void
pluginThreadLocalThing(IThreadPool* p) noexcept
{
    Arena* pArena = p->arena();
    ArenaPushScope pushed {pArena};

    Span sp {pArena->zallocV<char>(101), 100};
    const isize n = print::toSpan(sp, "hello from threadId: {}\n", p->threadId());
    LogInfo{StringView{sp.data(), n}};
}

#undef PLUGIN_SOURCE
