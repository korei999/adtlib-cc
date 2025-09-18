#include "adt/logs.hh"
#include "adt/ArenaList.hh"
#include "adt/Vec.hh"
#include "adt/ReverseIt.hh"
#include "adt/defer.hh"
#include "adt/StdAllocator.hh"
#include "adt/sort.hh"
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/Timer.hh"
#include "adt/assert.hh"
#include "adt/Logger.hh"

#include "Types.hh"

#include <vector>
#include <memory>

using namespace adt;

const isize BIG = 10000000;

int
main()
{
    {
        VecM<std::unique_ptr<Virtual>> v0;
        defer(
            v0.destroy()
        );

        v0.emplace(new V0 {1});
        v0.emplace(new V0 {2});
        v0.emplace(new V0 {3});
        v0.emplace(new V0 {4});

        for (auto& e : v0)
            CERR("{}, ", static_cast<const V0&>(*e));
        CERR("\n");
    }

    {
        VecManaged<What> v0;
        defer( v0.destroy() );

        v0.emplace(1);
        v0.emplace(2);
        v0.emplace(3);
        v0.emplace(4);
        v0.pop();
        What w = v0.pop();

        CERR("v0: {}, w: {}\n", v0, w);
    }

    CERR("\n");

    {
        VecManaged<Move> v0;
        defer( v0.destroy() );

        v0.emplace(1);
        v0.emplace(2);
        v0.emplace(3);
        v0.emplace(4);

        CERR("v0: {}\n", v0);
    }

    {
        auto a = alignUp8(123);
        LOG("a: {}\n", a);

        ArenaList arena(SIZE_1K);
        defer( arena.freeAll() );

        Vec<int> vec {&arena};
        for (int i = 0; i < 100000; ++i)
            vec.push(&arena, i);
    }

    {
        VecManaged<int> v {};
        defer( v.destroy() );
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

        LOG_WARN("v: {}\n", v);
        v.pushSpanAt(4, aSpan);
        LOG_GOOD("v: {}\n", v);
        v.pushSpanAt(10, aSpan);
        LOG_NOTIFY("v: {}\n", v);
    }

    Vec<ArenaList> aArenas(StdAllocator::inst(), 1);
    defer( aArenas.destroy(StdAllocator::inst()) );

    aArenas.push(StdAllocator::inst(), {SIZE_1M});
    defer( aArenas[0].freeAll() );

    Vec<f64> vec(&aArenas[0]);

    for (auto what : vec)
        LOG("asdf {}\n", what);

    vec.push(&aArenas[0], 5.123);
    vec.push(&aArenas[0], 3.234);
    vec.push(&aArenas[0], -1.341);
    vec.push(&aArenas[0], 123.023);
    vec.push(&aArenas[0], -999.1023);
    vec.push(&aArenas[0], 2.123);
    vec.push(&aArenas[0], 1.31);
    vec.push(&aArenas[0], 23.987);
    vec.push(&aArenas[0], 200.123);
    vec.push(&aArenas[0], -20.999);
    vec.emplace(&aArenas[0], 110001.54123);

    {
        auto vec0 = vec.clone(&aArenas[0]);
        sort::quick(&vec0);
        print::out("vec0: {}\n", vec0);
        ADT_ASSERT_ALWAYS(sort::sorted(vec0), "");
    }

    {
        auto vec1 = vec.clone(&aArenas[0]);
        sort::quick(&vec1, utils::ComparatorRev<f64> {});
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
        virtual  ~A() {}

        virtual void hi() {};
    };
    struct B : A
    {
        B(int _i, int _i1) : A(_i + _i1) {}
        virtual ~B() override {}

        virtual void hi() override final {};
    };

    {
        ArenaList a(sizeof(u32) * BIG);
        Vec<B> vec(&a, 77);
        for (u32 i = 0; i < BIG / 4; ++i)
            vec.push(&a, {(int)i, 0});
        a.freeAll();
    }

    {
        Timer timer {INIT};

        StdAllocator a;

        Vec<B> vec(&a);
        for (u32 i = 0; i < BIG; ++i)
            vec.emplace(&a, i, i);

        LOG("adt: {:.3} ms\n", timer.sElapsed() * 1000.0);

        vec.destroy(&a);
    }

    {
        Timer timer {INIT};

        std::vector<B> stdvec;
        /*stdvec.reserve(big);*/
        for (u32 i = 0; i < BIG; ++i)
            stdvec.emplace_back(i, i);

        LOG("std: {:.3} ms\n", timer.sElapsed() * 1000.0);
    }

    {
        VecManaged<int> v {};
        defer( v.destroy() );

        sort::push<sort::ORDER::INC>(&v, -15);
        sort::push<sort::ORDER::INC>(&v, 23);
        sort::push<sort::ORDER::INC>(&v, 999);
        sort::push<sort::ORDER::INC>(&v, -666);

        ADT_ASSERT_ALWAYS(sort::sorted(v.data(), v.size(), sort::ORDER::INC), "");

        LOG("sorted: {}\n", v);
    }

    {
        VecManaged<int> v {5, 666};
        defer( v.destroy() );
        for (int i = 0; i < 5; ++i)
            v.push(i);

        LOG("managed: {}\n", v);
    }
}
