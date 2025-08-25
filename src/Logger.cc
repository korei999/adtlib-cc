#include "adt/Logger.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/math.hh" /* IWYU pragma: keep */

#include <dlfcn.h>

using namespace adt;

static void (*pluginLoggingFunc)() noexcept;
static void (*pluginInit)(ILogger* p) noexcept;

static Logger s_logger;

atomic::Num<i64> s_i;

int
main(int argc, char** argv)
{
    LOG_NOTIFY("Logger test...\n");

    constexpr isize BIG = 30;

    void* pSo = dlopen("build/src/libLoggerUser.so", RTLD_NOW | RTLD_LOCAL);
    defer( dlclose(pSo) );
    pluginLoggingFunc = (decltype(pluginLoggingFunc))dlsym(pSo, "pluginLoggingFunc");
    pluginInit = (decltype(pluginInit))dlsym(pSo, "pluginInit");

    {
        new(&s_logger) Logger{stderr, ILogger::LEVEL::DEBUG, 1024 * 3};
        ILogger::setGlobal(&s_logger);
        ILogger::setGlobal(&s_logger);
        ILogger::setGlobal(&s_logger);

        pluginInit(&s_logger);
        defer( s_logger.destroy() );

        ThreadPool tp {s_logger.m_q.cap()};
        defer( tp.destroy() );

        for (isize i = 0; i < BIG; ++i)
        {
            tp.addRetry([i] {
                LogInfo{"hello: {}, {}\n", i, math::V3{(f32)i + 0, (f32)i + 1, (f32)i + 2}};
                s_i.fetchAdd(1, atomic::ORDER::RELAXED);
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
