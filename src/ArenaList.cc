#include "adt/ArenaList.hh"

#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

using namespace adt;

static void
go()
{
    ArenaList arena {SIZE_1M};
    defer( arena.freeAll() );

    ADT_ASSERT_ALWAYS(arena.memoryUsed() == 0, "{}", arena.memoryUsed());

    {
        IArena::IScope arenaScope = arena.restoreAfterScope();

        Vec<int> v {&arena};
        for (isize i = 0; i < 5; ++i)
            v.push(&arena, i);
    }

    ADT_ASSERT_ALWAYS(arena.memoryUsed() == 0, "{}", arena.memoryUsed());

    {
        IArena::Scope arenaScope {&arena};
        char* pMem = arena.mallocV<char>(SIZE_1M*2);
        ::memset(pMem, 0, 123);
    }

    ADT_ASSERT_ALWAYS(arena.memoryUsed() == 0, "{}", arena.memoryUsed());
}

int
main()
{
    ThreadPool ztp {ArenaList{}, SIZE_1M * 64};
    defer( ztp.destroy() );
    IThreadPool::setGlobal(&ztp);

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    defer( logger.destroy() );
    ILogger::setGlobal(&logger);

    LogInfo{"ArenaList test...\n"};
    go();
    LogInfo{"ArenaList test passed.\n"};
}
