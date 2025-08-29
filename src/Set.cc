#include "adt/logs.hh"
#include "adt/Set.hh"
#include "adt/ArenaList.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("Set test...\n");

    ArenaList arena {SIZE_1K};
    defer( arena.freeAll() );

    {
        Set<int> s {&arena};

        s.insert(&arena, 4);
        s.insert(&arena, 5);
        s.insert(&arena, 1);
        s.insert(&arena, 1);
        s.insert(&arena, -666);

        s.emplace(&arena, 2);

        {
            auto f5 = s.search(5);
            ADT_ASSERT_ALWAYS(f5, "");
        }

        for (auto& key : s)
        {
            CERR("key: {}, idx: {}\n", key, s.idx(&key));
        }
    }
}
