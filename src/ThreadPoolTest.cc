#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"
#include "adt/ScratchBuffer.hh"

#include <atomic>

using namespace adt;

static std::atomic<int> i = 0;

int
main()
{
    ThreadPool<512> tp(StdAllocator::inst(), 100);

    auto inc = [&] {
        i.fetch_add(1, std::memory_order_relaxed);
    };

    const int NTASKS = 100;
    for (int i = 0; i < NTASKS; ++i)
        tp.addLambda(inc);

    auto log = [&] { LOG("HWAT: {}\n", NTASKS); };
    tp.addLambda(log);

    tp.wait();

    tp.addLambda(inc);
    tp.addLambda(inc);
    tp.addLambda(inc);
    tp.addLambda(inc);

    tp.destroy(StdAllocator::inst());

    LOG("destroyed: i: {}\n", i.load());
    ADT_ASSERT(i.load(std::memory_order_relaxed) == NTASKS + 4, " ");

    CERR("\n\n\n\n");
}
