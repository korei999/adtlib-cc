#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/sort.hh"

#include <cassert>

using namespace adt;

int
main()
{
    Arena arena(adt::SIZE_1K);
    defer( freeAll(&arena) );

    Vec<int> vec(&arena.super);

    VecPush(&vec, 5);
    VecPush(&vec, 3);
    VecPush(&vec, -1);
    VecPush(&vec, 123);
    VecPush(&vec, -999);
    VecPush(&vec, 2);
    VecPush(&vec, 1);
    VecPush(&vec, 23);
    VecPush(&vec, 200);
    VecPush(&vec, -20);

    {
        auto vec0 = VecClone(&vec, &arena.super);
        sort::quick(&vec0.base);
        COUT("vec0: {}\n", vec0);
        assert(sort::sorted(vec0.base));
    }

    {
        auto vec1 = VecClone(&vec, &arena.super);
        sort::quick<VecBase, int, utils::compareRev>(&vec1.base);
        COUT("vec0: {}\n", vec1);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));
    }
}
