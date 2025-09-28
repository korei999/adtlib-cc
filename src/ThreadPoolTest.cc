#include "adt/Gpa.hh" /* IWYU pragma: keep */
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/ThreadPool.hh" /* IWYU pragma: keep */
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/rng.hh" /* IWYU pragma: keep */
#include "adt/Logger.hh"

using namespace adt;

static atomic::Int i {0};

/*static f64*/
/*loadsOfWork()*/
/*{*/
/*    rng::PCG32 rn {u32(i.load(atomic::ORDER::RELAXED)) + 1};*/
/*    auto range = rn.nextInRange(100000000, 1000000000);*/
/**/
/*    f64 what = 1.1;*/
/*    for (isize _i = 1; _i < range; ++_i)*/
/*    {*/
/*        if (_i & 1) continue;*/
/*        what += _i;*/
/*    }*/
/*    return what;*/
/*}*/

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("ThreadPool test...\n");

    ThreadPool tp {512, SIZE_1G};
    defer( tp.destroy() );

    auto inc = [&] {
        return i.fetchAdd(1, atomic::ORDER::RELAXED);
    };

    const int NTASKS = 50;
    for (int i = 0; i < NTASKS; ++i)
        tp.addRetry(inc);

    tp.addRetry(inc);
    tp.addRetry(inc);
    tp.addRetry(inc);
    tp.addRetry(inc);

    tp.wait(true);

    {
        auto got = i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(got == NTASKS + 4, "expected: {}, got: {}, ({})", NTASKS + 4, got, i.load(atomic::ORDER::RELAXED));
    }

    LogInfo("ThreadPool test passed...\n");
}
