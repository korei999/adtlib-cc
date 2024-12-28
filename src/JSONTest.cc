#include "adt/Arena.hh"
// #include "adt/FreeList.hh"
// #include "adt/MutexArena.hh"
#include "adt/OsAllocator.hh"
#include "adt/defer.hh"
#include "adt/file.hh"
#include "adt/logs.hh"
#include "json/Parser.hh"

using namespace adt;

int
main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        COUT("usage: {} <path to json> [-p(print)|-e(json creation example)]\n", argv[0]);
        return 0;
    }

    /*FreeList al(SIZE_1G * 5);*/
    Arena al(SIZE_1G * 3);
    /*MutexArena al(SIZE_1G * 2);*/

    /*defer( al.freeAll() );*/

    Opt<String> o_sJson = file::load(&al, argv[1]);

    if (!o_sJson) return 1;
    /*defer( o_sJson.value().destroy(OsAllocatorGet()) );*/

    json::Parser p(&al);
    json::STATUS eRes = p.loadParse(o_sJson.value());
    if (eRes == json::STATUS::FAIL) LOG_WARN("json::Parser::loadAndParse() failed\n");

    if (argc >= 3 && "-p" == String(argv[2]))
        p.print(stdout);

    /*p.destroy();*/
    /*_FreeListPrintTree(&al, &arena.base);*/
}
