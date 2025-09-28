#include "adt/ArenaList.hh"
#include "adt/PoolAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/ReverseIt.hh"
#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/time.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#include "Types.hh"

#include <string>

using namespace adt;

int
main()
{
    ThreadPool ztp {SIZE_1G};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    Arena& arena = *ztp.arena();

    {
        PoolAllocator pool {sizeof(RBTree<long>::Node), 200};
        defer( pool.freeAll() );

        RBTree<long> tree;

        tree.emplace(&pool, false, 1L);
        tree.emplace(&pool, false, -1L);
        tree.emplace(&pool, false, 2L);
        tree.emplace(&pool, false, -2L);
        tree.emplace(&pool, false, -3L);
        tree.emplace(&pool, false, -6L);
        tree.emplace(&pool, false, 10L);
        tree.emplace(&pool, false, 22L);

        tree.removeAndFree(&pool, -3L);
        tree.removeAndFree(&pool, -6L);

        LogInfo("root: {}\n", *tree.root());

        RBTree<long>::printNodes(StdAllocator::inst(), tree.root(), stdout);

        LogDebug("sizeof(RBTree<Empty>::Node): {}\n", sizeof(RBTree<Empty>::Node));
    }

    {
        RBTree<String> rb0;

        PoolAllocator pool {sizeof(RBTree<String>::Node), 500};
        defer( pool.freeAll() );

        rb0.insert(&pool, false, {&arena, "Hello"});
        rb0.insert(&pool, false, {&arena, "Sailor"});

        rb0.insert(&pool, false, {&arena, "Thirdy"});
        rb0.insert(&pool, false, {&arena, "Seconds"});
        rb0.insert(&pool, false, {&arena, "To"});
        rb0.insert(&pool, false, {&arena, "Mars"});

        auto rb1 = rb0.release();

        RBTree<String>::printNodes(StdAllocator::inst(), rb0.root(), stdout);
        RBTree<String>::printNodes(StdAllocator::inst(), rb1.root(), stdout);
    }

    {
        RBTree<std::string> t0;

        t0.insert(&arena, false, "std::string");
        t0.insert(&arena, false, "inserts");
        t0.insert(&arena, false, "just");
        t0.insert(&arena, false, "fine");
        t0.insert(&arena, false, "long string should allocate                          1");

        static_assert(ConvertsToStringView<std::string>);
        static_assert(!ConvertsToStringView<ArenaList>);

        RBTree<std::string>::printNodes(StdAllocator::inst(), t0.root(), stdout);

        t0.destructElements();
    }

    {
        RBTree<Move> t0;
        t0.emplace(&arena, false, 1);
        t0.emplace(&arena, false, 2);
        t0.emplace(&arena, false, 3);

        RBTree<Move>::printNodes(&arena, t0.root(), stdout);

        t0.destructElements();
    }

    {
        RBTree<int> t0;
        for (isize i = 0; i < 10; ++i)
            t0.emplace(&arena, false, i);

        RBTree<int>::printNodes(&arena, t0.root(), stdout);

        LogDebug("t0: {}\n", t0);
        for (auto& e : ReverseIt(t0))
            LogDebug("{}, ", e);
        LogDebug("\n\n");
    }

    {
        RBTree<int> t0;
        defer( t0.destroy(StdAllocator::inst()) );

        for (isize i = 0; i < 5000000; ++i)
            t0.emplace(StdAllocator::inst(), false, i);

        {
            auto* pFound = RBTree<int>::traversePre(t0.root(), [&](RBTree<int>::Node* p)
                {
                    if (p->data() == 5) return true;
                    return false;
                }
            );
            ADT_ASSERT_ALWAYS(pFound && pFound->data() == 5, "pFive: {}, data: {}", pFound, pFound ? pFound->data() : 0);
        }

        {
            auto* pFound = RBTree<int>::traversePost(t0.root(), [&](RBTree<int>::Node* p)
                {
                    if (p->data() == 2315) return true;
                    return false;
                }
            );
            ADT_ASSERT_ALWAYS(pFound && pFound->data() == 2315, "pFive: {}, data: {}", pFound, pFound ? pFound->data() : 0);
        }

        {
            auto* pFound = RBTree<int>::traverseIn(t0.root(), [&](RBTree<int>::Node* p)
                {
                    if (p->data() == 3235823) return true;
                    return false;
                }
            );
            ADT_ASSERT_ALWAYS(pFound && pFound->data() == 3235823, "pFive: {}, data: {}", pFound, pFound ? pFound->data() : 0);
        }

        {
            time::Clock timer {INIT};
            isize total = 0;
            for (auto& e : t0) total += e;

            LogDebug("for loop: time: {:.3} ms, totalSum: {}\n", timer.elapsedSec(), total);
            LogDebug("\n");
        }

        {
            time::Clock timer {INIT};
            isize total = 0;
            RBTree<int>::traverseIn(t0.root(), [&](RBTree<int>::Node* p)
                {
                    total += p->data();
                    return false;
                }
            );

            LogDebug("traverseIn: time: {:.3} ms, totalSum: {}\n", timer.elapsedSec(), total);
            LogDebug("\n");
        }

        {
            time::Clock timer {INIT};
            isize total = 0;
            RBTree<int>::traversePost(t0.root(), [&](RBTree<int>::Node* p)
                {
                    total += p->data();
                    return false;
                }
            );
            LogDebug("traversePost: time: {:.3} ms, totalSum: {}\n", timer.elapsedSec(), total);
            LogDebug("\n");
        }

        {
            time::Clock timer {INIT};
            isize total = 0;
            RBTree<int>::traversePre(t0.root(), [&](RBTree<int>::Node* p)
                {
                    total += p->data();
                    return false;
                }
            );
            LogDebug("traversePre: time: {:.3} ms, totalSum: {}\n", timer.elapsedSec(), total);
            LogDebug("\n");
        }
    }
}
