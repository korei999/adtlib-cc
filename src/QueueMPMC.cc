#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

#include "WIP/QueueMPMC.hh"

using namespace adt;

constexpr isize BIG = 1 << 18;

static QueueMPMC<int> s_q {BIG * 2};
static atomic::Int s_atomCounter {};

int
main()
{
    defer( s_q.destroy() );

    ThreadPool tp {Arena{}, 128, SIZE_1G*8};
    IThreadPool::setGlobal(&tp);
    defer( tp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("QueueMPMC test...\n");

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
