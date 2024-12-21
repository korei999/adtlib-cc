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

    Heap<long> h(&arena);
    defer( h.destroy(&arena) );

    h.pushMax(&arena, 2);
    h.pushMax(&arena, 4);
    h.pushMax(&arena, 1);
    h.pushMax(&arena, -2);
    h.pushMax(&arena, 8);

    LOG("h: [{}]\n", h.m_vec);

    while (h.m_vec.getSize() > 0)
        LOG("max: {}\n", h.maxExtract());
}
