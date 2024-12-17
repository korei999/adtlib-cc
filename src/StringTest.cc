#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/String.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    String sHello = StringAlloc(&arena.super, "hello");
}
