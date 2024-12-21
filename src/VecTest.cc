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

    Vec<f64> vec(&arena);

    vec.push(5.0);
    vec.push(3.0);
    vec.push(-1.0);
    vec.push(123.0);
    vec.push(-999.0);
    vec.push(2.0);
    vec.push(1.0);
    vec.push(23.0);
    vec.push(200.0);
    vec.push(-20.0);

    {
        auto vec0 = vec.clone(&arena);
        sort::quick(&vec0.base);
        COUT("vec0: {}\n", vec0);
        assert(sort::sorted(vec0.base));
    }

    {
        auto vec1 = vec.clone(&arena);
        sort::quick<VecBase, f64, utils::compareRev>(&vec1.base);
        COUT("vec0: {}\n", vec1);
        assert(sort::sorted(vec1.base, sort::ORDER::DEC));
    }
}
