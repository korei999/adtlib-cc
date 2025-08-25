#include "adt/Logger.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/math.hh" /* IWYU pragma: keep */

using namespace adt;

static Logger s_logger;

atomic::Int s_i;

int
main(int argc, char** argv)
{
    LOG_NOTIFY("Logger test...\n");

    constexpr isize BIG = 3000;

    {
        new(&s_logger) Logger{stderr, ILogger::LEVEL::DEBUG, 1024 * 3};
        defer( s_logger.destroy() );

        ThreadPool tp {StdAllocator::inst(), s_logger.m_q.cap()};
        defer( tp.destroy(StdAllocator::inst()) );

        for (isize i = 0; i < BIG; ++i)
        {
            tp.addRetry([i] {
                LogDebug{&s_logger, "hello: {}, {}\n", i, math::V3{(f32)i + 0, (f32)i + 1, (f32)i + 2}};
                s_i.fetchAdd(1, atomic::ORDER::RELAXED);
            });
        }
    }

    {
        auto i = s_i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(i == BIG, "s_i: {}", i);
    }

    LOG_GOOD("Logger test passed.\n");
}
