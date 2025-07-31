#include "adt/logs.hh"
#include "adt/SplayTree.hh"
#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/BufferAllocator.hh"

using namespace adt;

static char s_aBuff[SIZE_1M];

int
main()
{
    BufferAllocator tmpBuff {s_aBuff};

    LOG_NOTIFY("SplayTree test...\n");

    Arena arena {SIZE_1K};
    defer( arena.freeAll() );

    {
        SplayTree<int> st {};

        st.insert(&arena, 0);
        st.insert(&arena, 1);
        st.insert(&arena, 2);
        auto three = st.insert(&arena, -1);
        /*auto three = st.insert(&arena, -2);*/

        LOG_WARN("three: {}\n", three);

        /*st.insert(&arena, -2);*/
        /*st.insert(&arena, 10);*/
        /*st.insert(&arena, -1);*/
        /*st.insert(&arena, 22);*/
        /*st.insert(&arena, 2);*/
        /*st.insert(&arena, 1);*/
        /*st.insert(&arena, -21);*/

        st.print(&arena, stdout);
    }

    LOG_GOOD("SplayTree test passed.\n");
}
