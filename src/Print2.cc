#include "adt/Arena.hh"
#include "adt/Array.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#include "WIP/print2.hh"

using namespace adt;

static void
go()
{
    char aBuff[20]; memset(aBuff, '&', sizeof(aBuff));
    isize n = 0;
    n = print2::toSpan(aBuff,
        // "sv: '{:{} >{} f{}}'",
        // 4, 13, '+', StringView{"HELLO BIDEN"}
        "float: '{:15 .{} < f{}}'",
        5, '^', 12.12
    );
    // ADT_ASSERT(aBuff[n] == '\0', "{}", aBuff[n]);

    print::out("{}", aBuff);
    print::out("\nn: {}\n", n);
}

int
main()
{

    ThreadPool ztp {Arena{}, SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Print2 test...\n"};
    try
    {
        go();
    }
    catch (std::exception& ex)
    {
        LogError{"{}\n", ex.what()};
    }
    LogInfo{"Print2 test done\n"};
}
