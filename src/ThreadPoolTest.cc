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
        auto _ = i.fetchAdd(1, atomic::ORDER::RELAXED);
    };

    const int NTASKS = 50;
    for (int i = 0; i < NTASKS; ++i)
        ADT_ASSERT_ALWAYS(tp.add(inc), "");

    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");

    tp.wait();

    {
        auto got = i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(got == NTASKS + 4, "expected: {}, got: {}, ({})", NTASKS + 4, got, i.load(atomic::ORDER::RELAXED));
    }
}
