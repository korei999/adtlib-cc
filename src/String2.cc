#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

using namespace adt;

static void
test()
{
    VStringM s0 {"ooh my"};
    defer( s0.destroy() );

    s0.push(' ');
    s0.push('h');
    s0.push('o');
    s0.push('n');
    s0.push('e');
    s0.push('y');
    ADT_ASSERT(StringView(s0) == "ooh my honey", "{}", s0);

    s0.push(" you got me");
    ADT_ASSERT(StringView(s0) == "ooh my honey you got me", "{}", s0);

    s0.push(" working day n night...");
    ADT_ASSERT(StringView(s0) == "ooh my honey you got me working day n night...", "{}", s0);

    LogInfo{"s0: ({}) '{}'\n", s0.size(), s0};

    s0.reallocWith("asdf\r\n\r\n");
    ADT_ASSERT(StringView(s0) == "asdf\r\n\r\n", "{}", s0);

    s0.removeNLEnd(true);
    ADT_ASSERT(StringView(s0) == "asdf", "{}", s0);

    LogInfo("s0: ({}) '{}'\n", s0.size(), s0);

    ADT_ASSERT(utils::compare(s0, s0) == 0, "{}", s0);

    s0.destroy();
    ADT_ASSERT(StringView(s0).empty(), "{}", s0);
}

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, 1 << 12};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogDebug{"String2 test...\n"};
    {
        test();
    }
    LogDebug{"String2 test passed\n"};
}
