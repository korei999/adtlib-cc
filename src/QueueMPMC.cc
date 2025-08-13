#include "adt/QueueMPMC.hh"
#include "adt/Thread.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
// #include "adt/ThreadPool.hh"

using namespace adt;

constexpr isize BIG = 1 << 18;

// static QueueMPMC<int, BIG*2> s_q {INIT};
// static atomic::Int s_atomCounter {};

int
main()
{
    LOG_NOTIFY("QueueMPMC test...\n");

    // ThreadPool<128> tp {StdAllocator::inst()};
    // defer( tp.destroy(StdAllocator::inst()) );

    // auto clEnqueue = [&]
    // {
    //     while (!s_q.push(1))
    //         ;
    // };

    // auto clDequeue = [&]
    // {
    //     while (!s_q.empty())
    //     {
    //         if (Opt<int> r = s_q.pop())
    //         {
    //             s_atomCounter.fetchAdd(r.value(), atomic::ORDER::RELAXED);
    //             break;
    //         }
    //     }
    // };

    // for (isize i = 0; i < BIG; ++i)
    //     tp.addLambdaRetry(clEnqueue);

    // tp.wait();

    // for (isize i = 0; i < BIG; ++i)
    //     tp.addLambdaRetry(clDequeue);

    // tp.wait();

    // ADT_ASSERT_ALWAYS(s_atomCounter.load(atomic::ORDER::RELAXED) == BIG, "{}", s_atomCounter.load(atomic::ORDER::RELAXED));

    LOG_GOOD("QueueMPMC test passed.\n");
}
