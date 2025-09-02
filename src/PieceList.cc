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

    pl.defragment();

    int i = 0;
    for (auto e : pl.m_lPieces)
        print::out("({}): '{}'\n", i++, e.view());
    print::out("\n");
}

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"PieceList test...\n"};
    {
        test();
    }
    LogInfo{"PieceList test passed\n"};
}
