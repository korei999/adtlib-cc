#include "adt/StdAllocator.hh" /* IWYU pragma: keep */
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/logs.hh" /* IWYU pragma: keep */
#include "adt/ThreadPool.hh" /* IWYU pragma: keep */
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/ScratchBuffer.hh" /* IWYU pragma: keep */
#include "adt/rng.hh" /* IWYU pragma: keep */

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
    ThreadPoolWithMemory<512> tp {StdAllocator::inst(), SIZE_1K};
    defer( tp.destroy(StdAllocator::inst()) );

    auto inc = [&] {
        return i.fetchAdd(1, atomic::ORDER::RELAXED);
    };

    const int NTASKS = 50;
    for (int i = 0; i < NTASKS; ++i)
        tp.addRetry(inc);

    tp.wait();

    IThreadPool::Future<int> f0 {&tp};
    IThreadPool::Future<int> f1 {&tp};
    IThreadPool::Future<int> f2 {&tp};
    IThreadPool::Future<int> f3 {&tp};

    tp.addRetry(&f0, inc);
    tp.addRetry(&f1, inc);
    tp.addRetry(&f2, inc);
    tp.addRetry(&f3, inc);

    auto i0 = f0.waitData();
    auto i1 = f1.waitData();
    auto i2 = f2.waitData();
    auto i3 = f3.waitData();
    LOG("{}, {}, {}, {}\n", i0, i1, i2, i3);

    {
        auto got = i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(got == NTASKS + 4, "expected: {}, got: {}, ({})", NTASKS + 4, got, i.load(atomic::ORDER::RELAXED));
    }
}
