#ifdef PLUGIN_SOURCE
#error "must be defined by the plugin source"
#endif

#define PLUGIN_SOURCE

#include "LoggerUser.hh"

#include "adt/Logger.hh"

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

#undef PLUGIN_SOURCE
