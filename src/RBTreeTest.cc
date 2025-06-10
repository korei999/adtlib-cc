#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/PoolAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/StdAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    PoolAllocator al(sizeof(RBNode<long>), SIZE_8K);
    defer( al.freeAll() );

    RBTree<long> tree;

    tree.emplace(&al, false, 1L);
    tree.emplace(&al, false, -1L);
    tree.emplace(&al, false, 2L);
    tree.emplace(&al, false, -2L);
    tree.emplace(&al, false, -3L);
    tree.emplace(&al, false, -6L);
    tree.emplace(&al, false, 10L);
    tree.emplace(&al, false, 22L);

    tree.removeAndFree(&al, -3L);
    tree.removeAndFree(&al, -6L);

    LOG_GOOD("root: {}\n", *tree.root());

    RBPrintNodes(StdAllocator::inst(), tree.root(), stdout);

    LOG("sizeof(RBNode<Empty>): {}\n", sizeof(RBNode<Empty>));
}
