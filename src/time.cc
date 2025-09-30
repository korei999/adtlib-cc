#include "adt/time.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {2, ILogger::LEVEL::DEBUG, 512, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    {
        LogDebug{"time test...\n"};

        auto timer = time::now();
        for (isize i = 0; i < 2000000; ++i)
        {
            char aBuff[512] {};
            print::toSpan(aBuff, "{}, {}, {}", i, float(i), double(i));
        }
        auto now = time::now();
        LogInfo{"elapsed: {:.3} sec (raw: {})\n", time::diffSec(now, timer), time::diff(now, timer)};

        LogDebug{"time test passed\n"};
    }
}
