#include "adt/Arena.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    ChunkAllocator al(sizeof(RBNode<long>), SIZE_8K);
    defer( al.freeAll() );

    RBTree<long> tree(&al.super);
    defer( tree.destroy() );

    tree.insert(1L, false);
    tree.insert(-1L, false);
    tree.insert(2L, false);
    tree.insert(-2L, false);
    tree.insert(-3L, false);
    tree.insert(-6L, false);
    tree.insert(10L, false);
    tree.insert(22L, false);

    RBPrintNodes(&arena.super, tree.getRoot(), stdout, "", false);

    /*LOG("node: {}\n", *tree.getRoot());*/
}
