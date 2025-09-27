#include "WIP/PieceList.hh"

#include "adt/Logger.hh"
#include "adt/Arena.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

static void
test()
{
    Arena& arena = *IThreadPool::inst()->arena();

    auto rcp = RefCountedPtr<VStringM>::allocWithDeleter([](VStringM* p) { p->destroy(); }, "HelloWorld");
    defer( rcp.unref() );

    PieceList pl {rcp};
    defer( pl.destroy() );

    pl.insert(5, "|INSERT|");
    pl.insert(9, "<--->");
    pl.insert(0, "(+)");
    pl.insert(pl.size(), "[*]");
    pl.insert(pl.size() - 2, "{^}");

    {
        ArenaScope sg {&arena};
        VString s = pl.toString(&arena);
        LogDebug("s: '{}'\n", s);
        ADT_ASSERT_ALWAYS(StringView(s) == "(+)Hello|INS<--->ERT|World[{^}*]", "s(size: {}, cap: {}): '{}'", s.size(), s.cap(), s);
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
        ArenaScope sg {&arena};
        VString sDefragmented = pl.toString(&arena);
        LogInfo("({}): sDefragmented: '{}'\n", sDefragmented.size(), sDefragmented);
        ADT_ASSERT_ALWAYS(StringView(sDefragmented) == "(rld[{^}*]", "");
    }

    {
        PieceList::Node* p = pl.insert(4, "|%|");
        pl.insert(1, p->data.m_size - 1, p);
    }

    i = 0;
    for (auto& e : pl.m_lPieces)
        LogInfo("(4, |%|: ({}, {}): '{}'\n", i++, e.m_size, e.view());

    {
        ArenaScope sg {&arena};
        VString sDefragmented = pl.toString(&arena);
        LogInfo("({}): sDefragmented: '{}'\n", sDefragmented.size(), sDefragmented);
        ADT_ASSERT_ALWAYS(StringView(sDefragmented) == "(|%rld|%|[{^}*]", "sDefragmented: '{}'", sDefragmented);
    }

    pl.remove(1, pl.size() - 2);

    {
        ArenaScope sg {&arena};
        VString s = pl.toString(&arena);
        LogInfo("({}): s: '{}'\n", s.size(), s);
        ADT_ASSERT_ALWAYS(StringView(s) == "(]", "s: '{}'", s);
    }

    pl.remove(0, pl.size());

    ADT_ASSERT_ALWAYS(pl.size() == 0 && pl.empty(), "size: {}, empty: {}", pl.size(), pl.empty());
}

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1 << 12, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogWarn{"PieceList test...\n"};
    {
        test();
    }
    LogWarn{"PieceList test passed\n"};
}
