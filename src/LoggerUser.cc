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
    auto pLogger = adt::ILogger::inst();
    LogWarn("hello from 'LoggerUser'\n");
}
