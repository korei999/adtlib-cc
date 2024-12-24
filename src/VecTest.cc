#include "adt/FreeList.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"
#include "adt/sort.hh"

#include <cassert>

#include <vector>

using namespace adt;

int
main()
{
    VecBase<Arena> aArenas(OsAllocatorGet(), 1);
    defer( aArenas.destroy(OsAllocatorGet()) );

    aArenas.push(OsAllocatorGet(), SIZE_1M);
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

    const u32 big = 100000000;

    struct A
    {
        int i = 0;

        A(int _i) : i(_i) {}
        virtual  ~A() {};

        virtual void hi() {};
    };
    struct B : A
    {
        B(int _i) : A(_i) {}
        virtual ~B() override {}

        virtual void hi() override final {};
    };

    {
        Arena a(sizeof(u32) * big);
        Vec<B> vec(&a, 77);
        for (u32 i = 0; i < big / 4; ++i)
            vec.push(i);
        a.freeAll();
    }

    {
        f64 t0 = utils::timeNowMS();

        /* why arena is slower??? */
        FreeList a(sizeof(u32) * big);
        /*Arena a(nextPowerOf2(sizeof(u32) * big));*/

        Vec<B> vec(&a);
        for (u32 i = 0; i < big; ++i)
            vec.push(i);

        f64 t1 = utils::timeNowMS();
        LOG("adt: {} ms\n", t1 - t0);

        a.freeAll();
    }

    {
        f64 t0 = utils::timeNowMS();

        std::vector<B> stdvec;
        /*stdvec.reserve(big);*/
        for (u32 i = 0; i < big; ++i)
            stdvec.push_back(i);

        f64 t1 = utils::timeNowMS();
        LOG("std: {} ms\n", t1 - t0);
    }
}
