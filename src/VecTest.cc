#include "adt/FreeList.hh" /* IWYU pragma: keep */
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/ReverseIt.hh"
#include "adt/defer.hh"
#include "adt/StdAllocator.hh"
#include "adt/sort.hh"
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */

#include <vector>

using namespace adt;

const isize BIG = 10000000;

int
main()
{
    {
        auto a = alignUp8(123);
        LOG("a: {}\n", a);

        Arena arena(SIZE_1K);
        defer( arena.freeAll() );

        VecManaged<int> vec {&arena};
        for (int i = 0; i < 100000; ++i)
            vec.push(i);
    }

    Vec<Arena> aArenas(StdAllocator::inst(), 1);
    defer( aArenas.destroy(StdAllocator::inst()) );

    aArenas.push(StdAllocator::inst(), {SIZE_1M});
    defer( aArenas[0].freeAll() );

    VecManaged<f64> vec(&aArenas[0]);

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
        print::out("vec0: {}\n", vec0);
        ADT_ASSERT_ALWAYS(sort::sorted(vec0), "");
    }

    {
        auto vec1 = vec.clone(&aArenas[0]);
        sort::quick<Vec, f64, utils::compareRev>(&vec1);
        ADT_ASSERT_ALWAYS(sort::sorted(vec1, sort::ORDER::DEC), "");

        print::out("vec1(sorted):           {}\n", vec1);
        vec1.removeAndShift(2);
        print::out("vec1(removeAndShift(2): {}\n", vec1);

        print::out("vec1(rev): ");
        for (auto e : ReverseIt(vec1))
            print::out("{}, ", e);
        print::out("\n");

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
        VecManaged<B> vec(&a, 77);
        for (u32 i = 0; i < BIG / 4; ++i)
            vec.push({(int)i, 0});
        a.freeAll();
    }

    {
        f64 t0 = utils::timeNowMS();

        StdAllocator a;

        VecManaged<B> vec(&a);
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

    {
        VecManaged<int> v {StdAllocator::inst()};
        v.push(0);
        v.push(1);
        v.push(2);
        v.push(3);
        v.push(4);

        v.pushAt(0, 666);
        v.pushAt(3, 666);
        v.pushAt(5, 666);
        v.pushAt(6, 666);

        ADT_ASSERT_ALWAYS(v[0] == 666, "");
        ADT_ASSERT_ALWAYS(v[1] == 0, "");
        ADT_ASSERT_ALWAYS(v[2] == 1, "");
        ADT_ASSERT_ALWAYS(v[3] == 666, "");
        ADT_ASSERT_ALWAYS(v[4] == 2, "");
        ADT_ASSERT_ALWAYS(v[5] == 666, "");
        ADT_ASSERT_ALWAYS(v[6] == 666, "");
        ADT_ASSERT_ALWAYS(v[7] == 3, "");
        ADT_ASSERT_ALWAYS(v[8] == 4, "");

        const int aSpan[] {999, 999, 999};

        LOG("v: [{}]\n", v);
        v.pushSpanAt(4, aSpan);
        LOG("v: [{}]\n", v);
    }
}
