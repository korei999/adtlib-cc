#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/StdAllocator.hh"
#include "adt/Array.hh"
#include "adt/Timer.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    constexpr isize SIZE = 10000000;

    VecManaged<int> v {SIZE};
    defer( v.destroy() );
    v.setSize(v.cap());

    for (isize i = 0; i < SIZE; ++i)
        v[i] = i + 1;

    Timer timer {INIT};
    for (int e : v)
    {
        isize f = utils::binarySearchI(v, e);
        ADT_ASSERT_ALWAYS(f != NPOS && f + 1 == e, "f: {}, f + 1: {}, e: {}", f, f + 1, e);
    }
    COUT("time: {}\n", timer.msElapsed());

    return 0;
}
