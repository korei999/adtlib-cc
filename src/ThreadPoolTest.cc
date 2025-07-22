#include "adt/StdAllocator.hh" /* IWYU pragma: keep */
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/logs.hh" /* IWYU pragma: keep */
#include "adt/ThreadPool.hh" /* IWYU pragma: keep */
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/ScratchBuffer.hh" /* IWYU pragma: keep */

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
    tp.addLambda(inc);

    Future<int> fut {INIT};
    Future<int> fut2 {INIT};

    auto clFut = [&]
    {
        fut.data() = 666;
        fut.signal();
    };

    auto clFut2 = [&]
    {
        fut2.data() = 999;
        fut2.signal();
    };

    const bool bAdded = tp.addLambdaRetry(clFut, 1);
    const bool bAdded2 = tp.addLambdaRetry(clFut2, 1);

    if (bAdded) LOG_GOOD("future: {}\n", fut.waitData());
    else LOG_BAD("future: failed to add\n");

    if (bAdded2) LOG_GOOD("future2: {}\n", fut2.waitData());
    else LOG_BAD("future2: failed to add\n");

    tp.destroy(StdAllocator::inst());

    LOG("destroyed: i: {}\n", i.load(atomic::ORDER::RELAXED));
    ADT_ASSERT(i.load(atomic::ORDER::RELAXED) == NTASKS + 4, " ");

    CERR("\n\n");
}
