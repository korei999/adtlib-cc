#include "adt/OsAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"
#include "adt/ScratchBuffer.hh"

using namespace adt;

static std::atomic<int> i = 0;

int
main()
{
    ThreadPool tp(OsAllocatorGet(), 100);

    auto inc = [&] {
        i.fetch_add(1, std::memory_order_relaxed);
    };

    const int NTASKS = 100;
    for (int i = 0; i < NTASKS; ++i)
        tp.add(inc);

    tp.add([&]{ LOG("HWAT: {}\n", NTASKS); });

    tp.wait();

    tp.add(inc);
    tp.add(inc);
    tp.add(inc);
    tp.add(inc);

    tp.destroy();

    LOG("destroyed: i: {}\n", i.load());
    ADT_ASSERT(i.load(std::memory_order_relaxed) == NTASKS + 4, " ");

    CERR("\n\n\n\n");
}
