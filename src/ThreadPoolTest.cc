#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"
#include "adt/ScratchBuffer.hh"

using namespace adt;

static atomic::Int i {};

int
main()
{
    ThreadPoolWithMemory<512> tp {StdAllocator::inst(), SIZE_1K};

    auto inc = [&] {
        i.fetchAdd(1, atomic::ORDER::RELAXED);
    };

    const int NTASKS = 100;
    for (int i = 0; i < NTASKS; ++i)
        tp.addLambda(inc);

    auto log = [&] { LOG("HWAT: {}\n", NTASKS); };
    tp.addLambda(log);
    tp.addLambda(log);

    tp.wait();

    tp.addLambda(inc);
    tp.addLambda(inc);
    tp.addLambda(inc);
    tp.addLambdaRetry(inc);

    Future<int> fut {INIT};

    auto clFut = [&]
    {
        fut.data() = 666;
        fut.signal();
    };

    tp.addLambdaRetry(clFut);

    LOG_GOOD("future: {}\n", fut.waitData());

    tp.destroy(StdAllocator::inst());

    LOG("destroyed: i: {}\n", i.load(atomic::ORDER::RELAXED));
    ADT_ASSERT(i.load(atomic::ORDER::RELAXED) == NTASKS + 4, " ");

    CERR("\n\n\n\n");
}
