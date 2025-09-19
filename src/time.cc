#include "adt/Timer.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 512, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    {
        LogDebug{"time test...\n"};

        Timer timer {INIT};
        for (isize i = 0; i < 1000000; ++i)
        {
            char aBuff[512] {};
            print::toSpan(aBuff, "{}, {}, {}", i, float(i), double(i));
        }
        LogInfo{"elapsed: {:.3} sec\n", timer.elapsedSec()};

        LogDebug{"time test passed\n"};
    }
}
