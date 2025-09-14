#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

static void
usesThreadLocalArena(Arena* pArena)
{
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
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"ThreadPool test...\n"};
    {

        ThreadPool tp {1 << 11, SIZE_8G};
        defer( tp.destroy() );

        for (isize i = 0; i < 10000; ++i)
        {
            tp.addRetry([&] {
                {
                    ArenaPushScope pushed {tp.arena()};
                    usesThreadLocalArena(tp.arena());
                }
                ADT_ASSERT_ALWAYS(tp.arena()->memoryUsed() == 0, "{}", tp.arena()->memoryUsed());
            });
        }

        tp.wait(true);
    }
    LogInfo{"ThreadPool test passed\n"};
}
