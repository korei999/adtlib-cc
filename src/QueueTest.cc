#include "adt/OsAllocator.hh"
#include "adt/logs.hh"
#include "adt/defer.hh"
#include "adt/Arena.hh"
#include "adt/Queue.hh"

using namespace adt;

int
main()
{
    Arena arena(OsAllocatorGet(), SIZE_1K);
    defer( arena.freeAll() );

    Queue<long> q(&arena);

    q.pushBack(1);
    q.pushBack(2);
    q.pushBack(3);
    q.pushBack(6);
    q.pushBack(9);

    q.pushFront(-1);
    q.pushFront(-2);

    LOG("q: [{}]\n", q);
}
