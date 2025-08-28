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
            Vec<isize> v {};

            for (isize i = 0; i < 10000; ++i)
                v.push(&arena, i);

            print::toFILE(&arena, stdout, "v: {}\n", v);
        }
    }
    catch (const RuntimeException& ex)
    {
        LogDebug("{}\n", ex.getMsg());
    }

    LogInfo{"Arena2 test passed\n"};
}
