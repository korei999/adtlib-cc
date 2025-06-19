#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/PoolAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

#include "Types.hh"

#include <string>

using namespace adt;

int
main()
{
    Arena arena {SIZE_1K};
    defer( arena.freeAll() );

    {
        PoolAllocator pool {sizeof(RBNode<long>), 200};
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

        LOG_GOOD("root: {}\n", *tree.root());

        RBPrintNodes(StdAllocator::inst(), tree.root(), stdout);

        LOG("sizeof(RBNode<Empty>): {}\n", sizeof(RBNode<Empty>));
    }

    {
        RBTree<String> rb0;

        PoolAllocator pool {sizeof(decltype(rb0)::NodeType), 500};
        defer( pool.freeAll() );

        rb0.insert(&pool, false, {&arena, "Hello"});
        rb0.insert(&pool, false, {&arena, "Sailor"});

        rb0.insert(&pool, false, {&arena, "Thirdy"});
        rb0.insert(&pool, false, {&arena, "Seconds"});
        rb0.insert(&pool, false, {&arena, "To"});
        rb0.insert(&pool, false, {&arena, "Mars"});

        auto rb1 = rb0.release();

        RBPrintNodes(StdAllocator::inst(), rb0.root(), stdout);
        RBPrintNodes(StdAllocator::inst(), rb1.root(), stdout);
    }

    {
        RBTree<std::string> t0;

        t0.insert(&arena, false, "std::string");
        t0.insert(&arena, false, "inserts");
        t0.insert(&arena, false, "just");
        t0.insert(&arena, false, "fine");
        t0.insert(&arena, false, "long string should allocate                          1");

        RBPrintNodes(StdAllocator::inst(), t0.root(), stdout);

        t0.destructNodes();
    }

    {
        RBTree<Move> t0;
        t0.emplace(&arena, false, 1);
        t0.emplace(&arena, false, 2);
        t0.emplace(&arena, false, 3);

        RBPrintNodes(&arena, t0.root(), stdout);

        t0.destructNodes();
    }
}
