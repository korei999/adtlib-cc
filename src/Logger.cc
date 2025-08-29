#include "adt/Logger.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/math.hh" /* IWYU pragma: keep */

#ifdef _MSC_VER
#else
    #include <dlfcn.h>
#endif

using namespace adt;

static void (*pluginLoggingFunc)() noexcept;
static void (*pluginInit)(ILogger* p) noexcept;
static void (*pluginThreadLocalThing)(IThreadPoolWithMemory*) noexcept;

static Logger s_logger;

atomic::Num<i64> s_i;

int
main(int, char**)
{
    LOG_NOTIFY("Logger test...\n");

    constexpr isize BIG = 30;

#ifdef _MSC_VER
    HMODULE hMod = LoadLibraryA("build/src/LoggerUser.dll");
    ADT_ASSERT_ALWAYS(hMod != nullptr, "");
    defer( FreeLibrary(hMod) );
    pluginInit = (decltype(pluginInit))GetProcAddress((HMODULE)hMod, "pluginInit");
    pluginLoggingFunc = (decltype(pluginLoggingFunc))GetProcAddress((HMODULE)hMod, "pluginLoggingFunc");
    pluginThreadLocalThing = (decltype(pluginThreadLocalThing))GetProcAddress((HMODULE)hMod, "pluginThreadLocalThing");
#else
#ifdef __APPLE__
    void* pSo = dlopen("build/src/libLoggerUser.dylib", RTLD_NOW | RTLD_LOCAL);
#else
    void* pSo = dlopen("build/src/libLoggerUser.so", RTLD_NOW | RTLD_LOCAL);
#endif
    ADT_ASSERT_ALWAYS(pSo != nullptr, "");
    defer( dlclose(pSo) );
    pluginLoggingFunc = (decltype(pluginLoggingFunc))dlsym(pSo, "pluginLoggingFunc");
    pluginInit = (decltype(pluginInit))dlsym(pSo, "pluginInit");
    pluginThreadLocalThing = (decltype(pluginThreadLocalThing))dlsym(pSo, "pluginThreadLocalThing");
#endif

    {
        new(&s_logger) Logger{stderr, ILogger::LEVEL::DEBUG, 1024 * 3};
        ILogger::setGlobal(&s_logger);
        pluginInit(&s_logger);
        defer( s_logger.destroy() );

        ThreadPoolWithMemory tp {s_logger.m_q.cap(), SIZE_1K*4};
        defer( tp.destroy() );

        for (isize i = 0; i < BIG; ++i)
        {
            tp.addRetry([i, &tp] {
                LogInfo{"hello: {}, {}\n", i, math::V3{(f32)i + 0, (f32)i + 1, (f32)i + 2}};
                s_i.fetchAdd(1, atomic::ORDER::RELAXED);
                pluginThreadLocalThing(&tp);
            });
        }

        pluginLoggingFunc();
    }

    {
        atomic::Int::Type i = s_i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(i == BIG, "s_i: {}", i);
    }

    LOG_GOOD("Logger test passed.\n");
}
