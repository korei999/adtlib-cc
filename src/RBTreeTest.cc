#include "adt/logs.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    ChunkAllocator al(sizeof(RBNode<long>), SIZE_8K);
    RBTree<long> tree {};
    defer( tree.destroy() );

    tree.insert(1L, false);
    tree.insert(-1L, false);
    tree.insert(2L, false);
    tree.insert(-2L, false);
}
