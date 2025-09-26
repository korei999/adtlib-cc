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

constexpr isize BIG = 30000000;

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);

    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    Arena& arena = *ztp.arena();

    {
        Vec<int> v {&arena};
        static_assert(sizeof(v) == 32);
        v.push(0);
        v.push(1);
        v.push(2);
        v.push(3);
        v.push(4);
        v.push(5);
        v.push(6);

        for (isize i = 0; i < v.size(); ++i)
            ADT_ASSERT_ALWAYS(v[i] == i, "{}, {}", i, v[i]);
        LogDebug{"v: {}\n", v};

        auto vCopy0 {v};
        auto vMoved0 = std::move(vCopy0);
        Vec<int> vMoved1;
        vMoved1 = std::move(vMoved0);
        Vec<int> vCopy1 {vMoved1};
        Vec<int> vCopy2;
        vCopy2 = vCopy1;

        LogDebug{"vCopy0: {}, vMoved0: {}, vMoved1: {}, vCopy1: {}, vCopy2: {}\n", vCopy0, vMoved0, vMoved1, vCopy1, vCopy2};

        {
            VecM<int> v0 {};
            v0.push(0);
            v0.push(1);
            v0.push(2);
            v0.push(3);
            v0.push(4);
            v0.push(5);
            v0.push(6);

            VecM<int> v1 = std::move(v0);
            static_assert(sizeof(v1) == 24);
            LogDebug{"v0: {}, v1: {}\n", v0, v1};
        }
    }

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
            print::err("{}, ", static_cast<const V0&>(*e));
        print::err("\n");
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

        print::err("v0: {}, w: {}\n", v0, w);
    }

    print::err("\n");

    {
        VecManaged<Move> v0;
        defer( v0.destroy() );

        v0.emplace(1);
        v0.emplace(2);
        v0.emplace(3);
        v0.emplace(4);

        print::err("v0: {}\n", v0);
    }

    {
        auto a = alignUp8(123);
        print::err("a: {}\n", a);

        VecBase<int> vec {&arena};
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

        print::err("v: {}\n", v);
        v.pushSpanAt(4, aSpan);
        print::err("v: {}\n", v);
        v.pushSpanAt(10, aSpan);
        print::err("v: {}\n", v);
    }

    VecBase<ArenaList> aArenas(StdAllocator::inst(), 1);
    defer( aArenas.destroy(StdAllocator::inst()) );

    aArenas.push(StdAllocator::inst(), {SIZE_1M});
    defer( aArenas[0].freeAll() );

    VecBase<f64> vec(&aArenas[0]);

    for (auto what : vec)
        print::err("asdf {}\n", what);

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

    struct A : ForceMemcpyable
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
        VecBase<B> vec(&a, 77);
        for (u32 i = 0; i < BIG / 4; ++i)
            vec.push(&a, {(int)i, 0});
        a.freeAll();
    }

    {
        Timer timer {INIT};

        StdAllocator a;

        VecBase<B> vec(&a);
        for (isize i = 0; i < BIG; ++i)
            vec.emplace(&a, i, i);

        print::err("adt: {:.3} ms (cap: {})\n", timer.elapsedMSec(), vec.cap());

        vec.destroy(&a);
    }

    {
        Timer timer {INIT};

        std::vector<B> stdvec;
        /* Testing relocation performance. */

        /*stdvec.reserve(big);*/
        for (u32 i = 0; i < BIG; ++i)
            stdvec.emplace_back(i, i);

        print::err("std: {:.3} ms (cap: {})\n", timer.elapsedMSec(), stdvec.capacity());
    }

    {
        VecManaged<int> v {};
        defer( v.destroy() );

        sort::push<sort::ORDER::INC>(&v, -15);
        sort::push<sort::ORDER::INC>(&v, 23);
        sort::push<sort::ORDER::INC>(&v, 999);
        sort::push<sort::ORDER::INC>(&v, -666);

        ADT_ASSERT_ALWAYS(sort::sorted(v.data(), v.size(), sort::ORDER::INC), "");

        print::err("sorted: {}\n", v);
    }

    {
        VecManaged<int> v {5, 666};
        defer( v.destroy() );
        for (int i = 0; i < 5; ++i)
            v.push(i);

        print::err("managed: {}\n", v);
    }
}
