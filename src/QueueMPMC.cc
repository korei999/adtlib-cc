#include "adt/defer.hh"
#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

#include "WIP/QueueMPMC.hh"

using namespace adt;

constexpr isize BIG = 1 << 18;

static QueueMPMC<int, BIG*2> s_q {INIT};
static atomic::Int s_atomCounter {};

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("QueueMPMC test...\n");

    ThreadPool tp {128, SIZE_8G};
    defer( tp.destroy() );

    auto clEnqueue = [&]
    {
        while (!s_q.push(1))
            ;
    };

    auto clDequeue = [&]
    {
        while (!s_q.empty())
        {
            if (Opt<int> r = s_q.pop())
            {
                s_atomCounter.fetchAdd(r.value(), atomic::ORDER::RELAXED);
                break;
            }
        }
    };

    for (isize i = 0; i < BIG; ++i)
        tp.addRetry(clEnqueue);

    tp.wait(true);

    for (isize i = 0; i < BIG; ++i)
        tp.addRetry(clDequeue);

    tp.wait(true);

    ADT_ASSERT_ALWAYS(s_atomCounter.load(atomic::ORDER::RELAXED) == BIG, "{}", s_atomCounter.load(atomic::ORDER::RELAXED));

    LogInfo("QueueMPMC test passed.\n");
}
