#include "adt/FreeList.hh" /* IWYU pragma: keep */
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"
#include "adt/sort.hh"
#include "adt/FixedAllocator.hh"
#include "adt/ReverseIt.hh"

#include <cassert>

#include <vector>

using namespace adt;

const u32 BIG = 10000000;
static u8 s_bigMem[BIG * 16 + sizeof(FixedAllocator)] {};

int
main()
{
    VecBase<Arena> aArenas(OsAllocatorGet(), 1);
    defer( aArenas.destroy(OsAllocatorGet()) );

    aArenas.push(OsAllocatorGet(), {SIZE_1M});
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
        sort::quick<VecBase, f64, utils::compareRev<f64>>(&vec1.base);
        COUT("vec1: {}\n", vec1);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));

        COUT("vec1(rev): ");
        for (auto e : ReverseIt(vec1))
            COUT("{}, ", e);
        COUT("\n");
    }


    struct A
    {
        int i = 0;

        A(int _i) : i(_i) {}
        virtual  ~A() {};

        virtual void hi() {};
    };
    struct B : A
    {
        B(int _i, int _i1) : A(_i + _i1) {}
        virtual ~B() override {}

        virtual void hi() override final {};
    };

    {
        Arena a(sizeof(u32) * BIG);
        Vec<B> vec(&a, 77);
        for (u32 i = 0; i < BIG / 4; ++i)
            vec.push({(int)i, 0});
        a.freeAll();
    }

    {
        f64 t0 = utils::timeNowMS();

        /* why arena is slower??? */
        /*FreeList a(sizeof(u32) * BIG);*/
        /*FixedAllocator a(s_bigMem, sizeof(s_bigMem));*/
        OsAllocator a;
        /*Arena a(nextPowerOf2(sizeof(u32) * big));*/

        Vec<B> vec(&a);
        for (u32 i = 0; i < BIG; ++i)
            vec.emplace(i, i);

        f64 t1 = utils::timeNowMS();
        LOG("adt: {} ms\n", t1 - t0);

        vec.destroy();
    }

    {
        f64 t0 = utils::timeNowMS();

        std::vector<B> stdvec;
        /*stdvec.reserve(big);*/
        for (u32 i = 0; i < BIG; ++i)
            stdvec.emplace_back(i, i);

        f64 t1 = utils::timeNowMS();
        LOG("std: {} ms\n", t1 - t0);
    }
}
