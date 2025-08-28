#include "Arena2.hh"

#include "adt/Logger.hh"
#include "adt/Vec.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Arena2 test...\n"};

    try
    {
        Arena2 arena {SIZE_8G, 0};
        defer( arena.freeAll() );

        {
            Vec<int> v {};

            v.push(&arena, 0);
            v.push(&arena, 1);
            v.push(&arena, 2);
            v.push(&arena, 3);
            v.push(&arena, 4);

            LogDebug("v: {}\n", v);
        }
    }
    catch (const RuntimeException& ex)
    {
        LogDebug("{}\n", ex.getMsg());
    }

    LogInfo{"Arena2 test passed\n"};
}
