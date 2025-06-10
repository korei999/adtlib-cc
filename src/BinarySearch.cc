#include "adt/Vec.hh"
#include "adt/logs.hh"
#include "adt/StdAllocator.hh"
#include "adt/Array.hh"

using namespace adt;

int
main()
{
    constexpr isize SIZE = 10000000;

    VecManaged<int> v {SIZE};
    v.setSize(v.cap());
    for (isize i = 0; i < SIZE; ++i)
        v[i] = i + 1;

    const f64 t0 = utils::timeNowMS();
    for (int e : v)
    {
        isize f = utils::binarySearch(v, e);
        ADT_ASSERT_ALWAYS(f != NPOS && f + 1 == e, "f: {}, f + 1: {}, e: {}", f, f + 1, e);
    }
    const f64 t1 = utils::timeNowMS();
    COUT("time: {}\n", t1 - t0);

    return 0;
}
