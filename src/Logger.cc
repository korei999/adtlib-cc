#include "adt/Logger.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/math.hh"

using namespace adt;

static Logger s_logger;

int
main(int argc, char** argv)
{
    LOG_NOTIFY("Logger test...\n");

    {
        new(&s_logger) Logger{stderr, ILogger::LEVEL::DEBUG, 1024};
        defer( s_logger.destroy() );

        ThreadPool tp {StdAllocator::inst(), 1024};
        defer( tp.destroy(StdAllocator::inst()) );

        for (isize i = 0; i < 30; ++i)
        {
            tp.add([i] {
                LogDebug{&s_logger, "hello: {}, {}\n", i, math::V3{(f32)i + 0, (f32)i + 1, (f32)i + 2}};
            });
        }
    }

    LOG_GOOD("Logger test passed.\n");
}
