#include "adt/Arena.hh"
#include "adt/PoolAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    PoolAllocator al(sizeof(RBNode<long>), SIZE_8K);
    defer( al.freeAll() );

    RBTreeManaged<long> tree(&al);
    defer( tree.destroy() );

    tree.emplace(false, 1L);
    tree.emplace(false, -1L);
    tree.emplace(false, 2L);
    tree.emplace(false, -2L);
    tree.emplace(false, -3L);
    tree.emplace(false, -6L);
    tree.emplace(false, 10L);
    tree.emplace(false, 22L);

    tree.removeAndFree(-3L);
    tree.removeAndFree(-6L);

    LOG_GOOD("root: {}\n", *tree.getRoot());

    RBPrintNodes(&arena, tree.getRoot(), stdout);

    LOG("sizeof(RBNode<Empty>): {}\n", sizeof(RBNode<Empty>));
}
