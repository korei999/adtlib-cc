#include "adt/Arena.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"
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

    /*FreeList al(SIZE_1G * 2);*/
    Arena al(SIZE_1G * 2);
    /*MutexArena al(SIZE_1G * 2);*/

    /*OsAllocator al;*/

    LOG_BAD("v1: {}, v2: {}\n", sizeof(AllocatorVTable), sizeof(AllocatorVTableV2));

    defer( al.freeAll() );

    json::Parser p((IAllocator*)&al);
    json::RESULT eRes = json::ParserLoadParse(&p, argv[1]);
    if (eRes == json::FAIL) LOG_WARN("json::ParserLoadAndParse() failed\n");

    if (argc >= 3 && "-p" == String(argv[2]))
        json::ParserPrint(&p, stdout);

    /*json::ParserDestroy(&p);*/
    /*_FreeListPrintTree(&al, &arena.base);*/
}
