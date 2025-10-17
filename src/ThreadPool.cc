#include "adt/Arena.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

static void
usesThreadLocalArena()
{
    IArena* pArena = IThreadPool::inst()->arena();
    IArena::IScope arenaScope = pArena->restoreAfterScope();

    Vec<StringView> v {};
    for (isize i = 0; i < 10000; ++i)
    {
        Span sp = {pArena->zallocV<char>(100), 100};
        const isize n  = print::toSpan(sp, "{}, {}, {}", 1, 2.2, "HELLO");
        v.emplace(pArena, sp, n);
    }

    for (auto sv : v) ADT_ASSERT_ALWAYS(sv == "1, 2.2, HELLO", "{}", sv);
}

int
main()
{
    ThreadPool tp {Arena{}, 1 << 11, SIZE_1G*8};
    IThreadPool::setGlobal(&tp);
    defer( tp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"ThreadPool test...\n"};
    {

        for (isize i = 0; i < 10000; ++i)
        {
            tp.addRetry([] {
                usesThreadLocalArena();
                auto& tp = *IThreadPool::inst();
                ADT_ASSERT_ALWAYS(tp.arena()->memoryUsed() == 0, "{}", tp.arena()->memoryUsed());
            });
        }

        tp.wait(true);
    }
    LogInfo{"ThreadPool test passed\n"};
}
