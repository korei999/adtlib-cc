#include "adt/logs.hh"
#include "adt/Set.hh"
#include "adt/Arena.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("Set test...\n");

    Arena arena {SIZE_1K};
    defer( arena.freeAll() );

    {
        Set<int> s(&arena);
    }
}
