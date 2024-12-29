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
    defer( al.freeAll() );

    if (argc >= 1 && String(argv[1]) == "-e")
    {
        auto jObj = json::makeObject(&al, "obj0");
        jObj.pushToObject(&al, json::makeNumber("fifteen", 15));
        jObj.pushToObject(&al, json::makeFloat("fifteen.dot.fifteen", 15.15));
        u32 arrIdx = jObj.pushToObject(&al, json::makeArray(&al, "arr"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string0"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string1"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string2"));

        json::printNode(stdout, &jObj);
        putchar('\n');

        return 0;
    }

    Opt<String> o_sJson = file::load(&al, argv[1]);

    if (!o_sJson) return 1;
    /*defer( o_sJson.value().destroy(OsAllocatorGet()) );*/

    json::Parser p(&al);
    json::STATUS eRes = p.parse(o_sJson.value());
    if (eRes == json::STATUS::FAIL) LOG_WARN("json::Parser::parse() failed\n");

    if (argc >= 3 && "-p" == String(argv[2]))
        p.print(stdout);

    /*p.destroy();*/
    /*_FreeListPrintTree(&al, &arena.base);*/
}
