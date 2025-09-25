#include "adt/print.hh"
#include "adt/Thread.hh"
#include "adt/defer.hh"
#include "adt/Logger.hh"

using namespace adt;

static THREAD_STATUS
thread(void* pArg)
{
    if (pArg)
    {
        print::out("pArg: {}\n", *(int*)pArg);
        return *(THREAD_STATUS*)pArg;
    }
    else
    {
        print::out("pArg: {}\n", nullptr);
        return 0;
    }
}

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    int one = 1;

    {
        Thread thrd1(thread, &one);
        defer( thrd1.join() );

        Thread thrd2(thread, {});
        defer( thrd2.join() );
    }

    auto what = [] {
        LogDebug("what\n");
    };

    Thread thrd(what, Thread::ATTR::DETACHED);
    auto err = thrd.detach();
    LogDebug("err: {}\n", err);

    utils::sleepMS(100.0);

    LogDebug("one: {}\n", one);
}
