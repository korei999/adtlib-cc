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
        Set<int> s {&arena};

        auto sr4 = s.insert(&arena, 4);
        s.insert(&arena, 5);
        s.insert(&arena, 1);
        s.insert(&arena, 1);
        s.insert(&arena, -666);

        for (auto& key : s)
        {
            CERR("key: {}, idx: {}\n", key, s.idx(&key));
        }
    }
}
