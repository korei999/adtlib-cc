#include "adt/Logger.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

static Logger s_logger;

int
main(int argc, char** argv)
{
    LOG_NOTIFY("Logger test...\n");

    {
        new(&s_logger) Logger{ILogger::LEVEL::DEBUG, stderr, 1024};
        defer( s_logger.destroy() );

        ThreadPool tp {StdAllocator::inst(), 1024};
        defer( tp.destroy(StdAllocator::inst()) );

        for (isize i = 0; i < 30; ++i)
        {
            tp.add([i] {
                LogDebug(&s_logger, "hello: {}\n", i);
            });
        }
    }

    LOG_GOOD("Logger test passed.\n");
}
