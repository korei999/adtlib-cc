#include "adt/Vec.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/ThreadPool.hh"
#include "adt/math.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("ParallelFor test...\n");

    Arena arena {SIZE_1K * 3};
    defer( arena.freeAll() );

    ThreadPool<128> tp {&arena};
    defer( tp.destroy(&arena) );

    Vec<f32> v;
    defer( v.destroy(&arena) );

    for (isize i = 0; i < 43; ++i)
        v.push(&arena, i);

    Vec<Future<Span<f32>>*> vFutures = parallelFor(&arena, &tp, Span<f32> {v},
        [](Span<f32> spBatch, isize)
        {
            for (auto& e : spBatch) e += (6 * (e / 1.66f));
        }
    );

    for (auto* pF : vFutures) pF->wait();

    CERR("v: {}\n", v);

    for (isize i = 0; i < v.size(); ++i)
    {
        ADT_ASSERT_ALWAYS(math::eq(v[i], f32(i) + (6 * (f32(i) / 1.66f))), "v[{}]: {}, (expected: {})", i, v[i], f32(i) + (6 * (f32(i) / 1.66f)));
    }

    LOG_GOOD("ParallelFor test passed\n");
}
