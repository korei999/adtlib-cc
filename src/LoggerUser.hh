#pragma once

#ifdef _MSC_VER
    #ifdef PLUGIN_SOURCE
        #define PLUGIN_API extern "C" __declspec(dllexport)
    #else
        #define PLUGIN_API extern "C" __declspec(dllimport)
    #endif
#else
    #define PLUGIN_API extern "C" __attribute__((visibility("default")))
#endif

namespace adt
{
struct ILogger;
struct IThreadPoolWithMemory;
}

PLUGIN_API void pluginInit(adt::ILogger* p) noexcept;
PLUGIN_API void pluginLoggingFunc() noexcept;
PLUGIN_API void pluginThreadLocalThing(adt::IThreadPoolWithMemory*) noexcept;
