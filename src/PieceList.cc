#include "PieceList.hh"

#include "adt/Logger.hh"

using namespace adt;

static void
test()
{
    auto rcp = RefCountedPtr<StringM>::allocWithDeleter([](StringM* p) { p->destroy(); }, "HelloWorld");
    defer( rcp.unref() );

    PieceList pl {rcp};
    defer( pl.destroy() );

    pl.insert(5, "|INSERT|");
    pl.insert(9, "<--->");
    pl.insert(0, "(+)");
    pl.insert(pl.size(), "[*]");
    pl.insert(pl.size() - 2, "{^}");

    int i = 0;

    isize r0 = 1, r1 = 15;
    LogDebug("before remove ({}, {})\n", r0, r1);

    i = 0;
    for (auto e : pl.m_lPieces)
        LogInfo("({}, {}): '{}'\n", i++, e.m_size, e.view());

    pl.remove(r0, r1);

    LogDebug("after remove ({}, {})\n", r0, r1);
    i = 0;
    for (auto e : pl.m_lPieces)
        LogInfo("({}, {}): '{}'\n", i++, e.m_size, e.view());

    LogDebug("before defragment()\n");
    i = 0;
    for (auto e : pl.m_lPieces)
        LogInfo("({}, {}): '{}'\n", i++, e.m_size, e.view());

    pl.defragment();

    LogDebug("after defragment()\n");
    i = 0;
    for (auto e : pl.m_lPieces)
        LogInfo("defragmented: ({}, {}): '{}'\n", i++, e.m_size, e.view());
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
