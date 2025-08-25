#pragma once

#define PLUGIN_API extern "C" __attribute__((visibility("default")))

namespace adt
{
struct ILogger;
}

PLUGIN_API void pluginInit(adt::ILogger* p) noexcept;
PLUGIN_API void pluginLoggingFunc() noexcept;
