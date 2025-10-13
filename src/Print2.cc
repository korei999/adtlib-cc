#include "adt/Arena.hh"
#include "adt/Array.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"
#include "adt/SList.hh"

#include "WIP/print2.hh"

#include <string_view>

using namespace adt;

static void
go()
{
    Arena* pArena = dynamic_cast<Arena*>(IThreadPool::inst()->arena());
    ADT_ASSERT_ALWAYS(pArena != nullptr, "");

    IArena::Scope arenaScope {pArena};

    SListM<i64> lInts;
    lInts.insert(1);
    lInts.insert(2);
    lInts.insert(3);
    lInts.insert(4);

    char aBuff[128]; memset(aBuff, '&', sizeof(aBuff));
    isize n = 0;
    n = print2::toFILE(stdout, pArena, isize(0), "hello im toxic: '{:1 >10 f+}', initList: {:3 f&}, lInts: {}, std::string_view: '{}'\n",
        999, std::initializer_list{1, 2, 3, 4}, lInts, std::string_view{"std string view"}
    );
}

int
main()
{
    ThreadPool ztp {Arena{}, SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Print2 test...\n"};
    try
    {
        go();
    }
    catch (std::exception& ex)
    {
        LogError{"{}\n", ex.what()};
    }
    LogInfo{"Print2 test done\n"};
}
