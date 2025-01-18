#include "adt/types.hh"
#include "adt/Vec.hh"
#include "adt/Arena.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    Vec<int> vec(&arena);
    vec.push(1);
    vec.push(2);

    auto death = vec[2];
}
