#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Heap.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

}
