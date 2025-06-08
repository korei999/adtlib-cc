#include "adt/Span.hh"
#include "adt/types.hh"
#include "adt/Vec.hh"
#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/Span2D.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    VecPmr<int> vec {&arena};
    vec.push(1);
    vec.push(2);

    auto debugDeath = vec[2];
}
