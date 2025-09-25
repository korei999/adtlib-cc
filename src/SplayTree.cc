#include "adt/SplayTree.hh"
#include "adt/ArenaList.hh"
#include "adt/defer.hh"
#include "adt/BufferAllocator.hh"
#include "adt/Logger.hh"

using namespace adt;

static char s_aBuff[SIZE_1M];

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    BufferAllocator tmpBuff {s_aBuff};

    LogInfo("SplayTree test...\n");

    ArenaList arena {SIZE_1K};
    defer( arena.freeAll() );

    {
        SplayTree<int> st {};

        st.insert(&arena, -2);
        st.insert(&arena, 10);
        st.insert(&arena, -1);
        st.insert(&arena, -22);
        st.insert(&arena, 2);
        st.insert(&arena, 1);
        SplayNode<int>* pM21 = st.insert(&arena, -21);
        ADT_ASSERT_ALWAYS(pM21 == st.root(), "pM21: {}, root: {}", pM21, st.root());

        st.remove(pM21);

        {
            auto* p10 = st.search(10);
            ADT_ASSERT_ALWAYS(p10 && p10 == st.root(), "p10: {}, root: {}", (void*)p10, (void*)st.root());

            auto* pM21 = st.search(-21);
            ADT_ASSERT_ALWAYS(!pM21, "pM21: {}, root: {}", (void*)pM21, (void*)st.root());

            auto* p2 = st.search(2);
            ADT_ASSERT_ALWAYS(p2 && p2 == st.root(), "p2: {}, root: {}", (void*)p2, (void*)st.root());

            ADT_ASSERT_ALWAYS(p10 && p10->m_data == 10, "{}", p10);
            ADT_ASSERT_ALWAYS(p2 && p2->m_data == 2, "{}", p2);
        }

        st.print(&arena, stdout);
    }

    LogInfo("SplayTree test passed.\n");
}
