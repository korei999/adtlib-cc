#include "adt/Arena.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/OsAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    Arena arena(OsAllocatorGet(), SIZE_1K);
    defer( arena.freeAll() );

    ChunkAllocator al(OsAllocatorGet(), sizeof(RBNode<long>), SIZE_8K);
    defer( al.freeAll() );

    RBTree<long> tree(&al);
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
}
