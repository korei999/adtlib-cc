#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#ifdef _MSC_VER
#else
    #include <dlfcn.h>
#endif

#include "WIP/math.hh" /* IWYU pragma: keep */

using namespace adt;

static void (*pluginLoggingFunc)() noexcept;
static void (*pluginInit)(IThreadPool* pTp, ILogger* pLogger) noexcept;
static void (*pluginThreadLocalThing)() noexcept;

static Logger s_logger;

atomic::Num<i64> s_i;

int
main(int, char**)
{
    print::out("Logger test...\n");

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
        ThreadPool tp {1024, SIZE_1M * 64};
        IThreadPool::setGlobal(&tp);
        defer( tp.destroy() );

        new(&s_logger) Logger{2, ILogger::LEVEL::DEBUG, 1 << 10};
        ILogger::setGlobal(&s_logger);
        defer( s_logger.destroy() );

        pluginInit(&tp, &s_logger);

        for (isize i = 0; i < BIG; ++i)
        {
            tp.addRetry([i] {
                LogInfo{"hello: {}, {}\n", i, math::V3{{(f32)i + 0, (f32)i + 1, (f32)i + 2}}};
                s_i.fetchAdd(1, atomic::ORDER::RELAXED);
                pluginThreadLocalThing();
            });
        }

        pluginLoggingFunc();

        {
            LogDebug{"log1\n"};
            LogDebug{"log2\n"};
            LogDebug{"log3\n"};
            LogDebug{"log4\n"};
            LogDebug{"log5\n"};
        }
    }

    {
        atomic::Int::Type i = s_i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(i == BIG, "s_i: {}", i);
        print::out("i: {}\n", i);
    }

    print::out("Logger test passed.\n");
}
