#include "WIP/String2.hh"

#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

using namespace adt;

static void
test()
{
    auto* pStd = StdAllocator::inst();

    String2 s0 {pStd, "ooh my"};
    defer( s0.destroy(pStd) );

    s0.push(pStd, ' ');
    s0.push(pStd, 'h');
    s0.push(pStd, 'o');
    s0.push(pStd, 'n');
    s0.push(pStd, 'e');
    s0.push(pStd, 'y');
    ADT_ASSERT(StringView(s0) == "ooh my honey", "{}", s0);

    s0.push(pStd, " you got me");
    ADT_ASSERT(StringView(s0) == "ooh my honey you got me", "{}", s0);

    s0.push(pStd, " working day n night...");
    ADT_ASSERT(StringView(s0) == "ooh my honey you got me working day n night...", "{}", s0);

    LogInfo{"s0: ({}) '{}'\n", s0.size(), s0};

    s0.reallocWith(pStd, "asdf\r\n\r\n");
    ADT_ASSERT(StringView(s0) == "asdf\r\n\r\n", "{}", s0);

    s0.removeNLEnd(true);
    ADT_ASSERT(StringView(s0) == "asdf", "{}", s0);

    LogInfo("s0: ({}) '{}'\n", s0.size(), s0);
}

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1 << 12};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogDebug{"String2 test...\n"};
    {
        test();
    }
    LogDebug{"String2 test passed\n"};
}
