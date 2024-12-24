#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"
#include "adt/sort.hh"

#include <cassert>

using namespace adt;

int
main()
{
    VecBase<Arena> aArenas(OsAllocatorGet(), 1);
    defer( aArenas.destroy(OsAllocatorGet()) );

    aArenas.push(OsAllocatorGet(), SIZE_1K);
    defer( aArenas[0].freeAll() );

    Vec<f64> vec(&aArenas[0]);

    vec.push(5.0);
    vec.push(3.0);
    vec.push(-1.0);
    vec.push(123.0);
    vec.push(-999.0);
    vec.push(2.0);
    vec.push(1.0);
    vec.push(23.0);
    vec.push(200.0);
    vec.push(-20.0);

    {
        auto vec0 = vec.clone(&aArenas[0]);
        sort::quick(&vec0.base);
        COUT("vec0: {}\n", vec0);
        assert(sort::sorted(vec0.base));
    }

    {
        auto vec1 = vec.clone(&aArenas[0]);
        sort::quick<VecBase, f64, utils::compareRev>(&vec1.base);
        COUT("vec0: {}\n", vec1);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));
    }

    {
        f64 t0 = utils::timeNowMS();

        const u32 big = 100000000;
        Vec<u32> vec(&aArenas.first(), 77);
        for (u32 i = 0; i < big; ++i)
            vec.push(i);

        f64 t1 = utils::timeNowMS();
        LOG("{} ms\n", t1 - t0);
    }
}
