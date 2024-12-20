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

    Heap<long> h(&arena.super);
    defer( h.destroy(&arena.super) );

    h.pushMax(&arena.super, 2);
    h.pushMax(&arena.super, 4);
    h.pushMax(&arena.super, 1);
    h.pushMax(&arena.super, -2);
    h.pushMax(&arena.super, 8);

    LOG("h: [{}]\n", h.m_vec);

    while (h.m_vec.getSize() > 0)
        LOG("max: {}\n", h.maxExtract());
}
