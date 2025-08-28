#include "adt/FlatArena.hh"
#include "adt/Logger.hh"
#include "adt/Vec.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::NONE, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Arena2 test...\n"};

    try
    {
        FlatArena arena {SIZE_8G, 0};
        defer( arena.freeAll() );

        {
            Vec<isize> v0 {};
            Vec<isize> v1 {};

            for (isize i = 0; i < 100; ++i)
            {
                if (i & 1) v0.push(&arena, i);
                else v1.push(&arena, i);
            }

            for (auto e : v0) ADT_ASSERT_ALWAYS(e & 1, "e: {}", e);
            for (auto e : v1) ADT_ASSERT_ALWAYS(!(e & 1), "e: {}", e);

            print::toFILE(StdAllocator::inst(), stdout, "v0: {}\n", v0);
            print::toFILE(StdAllocator::inst(), stdout, "v1: {}\n", v1);
        }
    }
    catch (const RuntimeException& ex)
    {
        LogDebug("{}\n", ex.getMsg());
    }

    LogInfo{"Arena2 test passed\n"};
}
