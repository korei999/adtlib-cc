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
    defer( arena.freeAll() );

    Vec<int> vec(&arena.super);

    vec.push(5);
    vec.push(3);
    vec.push(-1);
    vec.push(123);
    vec.push(-999);
    vec.push(2);
    vec.push(1);
    vec.push(23);
    vec.push(200);
    vec.push(-20);

    {
        auto vec0 = vec.clone(&arena.super);
        sort::quick(&vec0.base);
        COUT("vec0: {}\n", vec0);
        assert(sort::sorted(vec0.base));
    }

    {
        auto vec1 = vec.clone(&arena.super);
        sort::quick<VecBase, int, utils::compareRev>(&vec1.base);
        COUT("vec0: {}\n", vec1);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));
    }
}
