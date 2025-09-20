#include "WIP/PieceList.hh"

#include "adt/Logger.hh"
#include "adt/Arena.hh"

using namespace adt;

static void
test()
{
    Arena arena {SIZE_1G};
    defer( arena.freeAll() );

    auto rcp = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, "HelloWorld");
    defer( rcp.unref() );

    PieceList pl {rcp};
    defer( pl.destroy() );

    pl.insert(5, "|INSERT|");
    pl.insert(9, "<--->");
    pl.insert(0, "(+)");
    pl.insert(pl.size(), "[*]");
    pl.insert(pl.size() - 2, "{^}");

    {
        ArenaPushScope sg {&arena};
        String s = pl.toString(&arena);
        LogDebug("s: '{}'\n", s);
        ADT_ASSERT_ALWAYS(s == "(+)Hello|INS<--->ERT|World[{^}*]", "s: '{}'", s);
    }

    int i = 0;

    isize r0 = 1, r1 = 22;
    LogDebug("before remove ({}, {})\n", r0, r1);

    i = 0;
    for (auto& e : pl.m_lPieces)
        LogInfo("({}, {}): '{}'\n", i++, e.m_size, e.view());

    pl.remove(r0, r1);

    LogDebug("after remove ({}, {})\n", r0, r1);
    i = 0;
    for (auto& e : pl.m_lPieces)
        LogInfo("({}, {}): '{}'\n", i++, e.m_size, e.view());

    pl.defragment();

    LogDebug("after defragment()\n");
    i = 0;
    for (auto& e : pl.m_lPieces)
        LogInfo("defragmented: ({}, {}): '{}'\n", i++, e.m_size, e.view());

    {
        ArenaPushScope sg {&arena};
        String sDefragmented = pl.toString(&arena);
        LogInfo("({}): sDefragmented: '{}'\n", sDefragmented.size(), sDefragmented);
        ADT_ASSERT_ALWAYS(sDefragmented == "(rld[{^}*]", "");
    }

    {
        PieceList::Node* p = pl.insert(4, "|%|");
        pl.insert(1, p->data.m_size - 1, p);
    }

    i = 0;
    for (auto& e : pl.m_lPieces)
        LogInfo("(4, |%|: ({}, {}): '{}'\n", i++, e.m_size, e.view());

    {
        ArenaPushScope sg {&arena};
        String sDefragmented = pl.toString(&arena);
        LogInfo("({}): sDefragmented: '{}'\n", sDefragmented.size(), sDefragmented);
        ADT_ASSERT_ALWAYS(sDefragmented == "(|%rld|%|[{^}*]", "sDefragmented: '{}'", sDefragmented);
    }

    pl.remove(1, pl.size() - 2);

    {
        ArenaPushScope sg {&arena};
        String s = pl.toString(&arena);
        LogInfo("({}): s: '{}'\n", s.size(), s);
        ADT_ASSERT_ALWAYS(s == "(]", "s: '{}'", s);
    }

    pl.remove(0, pl.size());

    ADT_ASSERT_ALWAYS(pl.size() == 0 && pl.empty(), "size: {}, empty: {}", pl.size(), pl.empty());
}

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogDebug{"PieceList test...\n"};
    {
        test();
    }
    LogDebug{"PieceList test passed\n"};
}
