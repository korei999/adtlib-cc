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
    defer( tp.destroy(StdAllocator::inst()) );

    auto inc = [&] {
        i.fetchAdd(1, atomic::ORDER::RELAXED);
    };

    const int NTASKS = 50;
    for (int i = 0; i < NTASKS; ++i)
        ADT_ASSERT_ALWAYS(tp.add(inc), "");

    auto log = [&] { LOG("HWAT: {}\n", NTASKS); };

    tp.wait();

    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");
    ADT_ASSERT_ALWAYS(tp.add(inc), "");

    tp.wait();

    {
        auto got = i.load(atomic::ORDER::RELAXED);
        ADT_ASSERT_ALWAYS(got == NTASKS + 4, "expected: {}, got: {}", NTASKS + 4, got);
    }

    CERR("\n\n");
}
