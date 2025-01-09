#include "adt/FreeList.hh" /* IWYU pragma: keep */
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"
#include "adt/sort.hh"
#include "adt/FixedAllocator.hh" /* IWYU pragma: keep */
#include "adt/ReverseIt.hh"

#include <cassert>

#include <vector>

using namespace adt;

const ssize BIG = 10000000;

int
main()
{
    {
        auto a = align8(123);
        LOG("a: {}\n", a);

        Arena arena(SIZE_1K);
        defer( arena.freeAll() );

        Vec<int> vec(&arena);
        for (int i = 0; i < 100000; ++i)
            vec.push(i);
    }

    VecBase<Arena> aArenas(OsAllocatorGet(), 1);
    defer( aArenas.destroy(OsAllocatorGet()) );

    aArenas.push(OsAllocatorGet(), {SIZE_1M});
    defer( aArenas[0].freeAll() );

    Vec<f64> vec(&aArenas[0]);

    for (auto what : vec)
        LOG("asdf {}\n", what);

    vec.push(5.123);
    vec.push(3.234);
    vec.push(-1.341);
    vec.push(123.023);
    vec.push(-999.1023);
    vec.push(2.123);
    vec.push(1.31);
    vec.push(23.987);
    vec.push(200.123);
    vec.push(-20.999);
    vec.emplace(110001.54123);

    {
        auto vec0 = vec.clone(&aArenas[0]);
        sort::quick(&vec0);
        /*sort::insertion(vec0.data(), 0, vec0.getSize() - 1);*/
        COUT("vec0: {}\n", vec0);
        assert(sort::sorted(vec0.base));
    }

    {
        auto vec1 = vec.clone(&aArenas[0]);
        sort::quick<VecBase, f64, utils::compareRev<f64>>(&vec1.base);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));

        COUT("vec1(sorted):           {}\n", vec1);
        vec1.removeAndShift(2);
        COUT("vec1(removeAndShift(2): {}\n", vec1);

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
