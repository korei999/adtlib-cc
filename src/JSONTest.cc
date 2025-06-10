#include "adt/Arena.hh"
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/FreeList.hh" /* IWYU pragma: keep */
#include "adt/StdAllocator.hh" /* IWYU pragma: keep */
#include "adt/Queue.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/file.hh"
#include "adt/logs.hh"
#include "json/Parser.hh"

// #include "adt/MiMalloc.hh" /* IWYU pragma: keep */

using namespace adt;

// [[maybe_unused]] static u8 s_aMem[SIZE_1G] {};

int
main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        COUT("usage: {} <path to json> [-p(print)|-e(json creation example)]\n", argv[0]);
        return 0;
    }

    if (argc >= 1 && StringView(argv[1]) == "-e")
    {
        Arena al(SIZE_1K);
        defer( al.freeAll() );

        auto jObj = json::makeObject(&al, ""); /* root object usually has no name, this json parser allows to have it */
        jObj.pushToObject(&al, json::makeNumber("fifteen", 15));
        jObj.pushToObject(&al, json::makeFloat("fifteen.dot.fifteen", 15.15));
        u32 arrIdx = jObj.pushToObject(&al, json::makeArray(&al, "arr"));
        /* array values have no keys, but this json parser makes no distiction between arrays and objects, so the keys are empty here */
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string0"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string1"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string2"));

        json::printNode(stdout, &jObj);
        putchar('\n');

        return 0;
    }

    try
    {
        // FreeList al(SIZE_1G);
        // StdAllocator al;
        // MiMalloc al;
        Arena al(SIZE_8M);
        // MiHeap al(0);
        defer( al.freeAll() );

        String sJson = file::load(&al, argv[1]);

        if (!sJson) return 1;

        // defer( sJson.destroy(&al) );

        json::Parser p {};
        bool eRes = p.parse(&al, sJson);
        if (!eRes) LOG_WARN("json::Parser::parse() failed\n");
        // defer( p.destroy() );

        if (argc >= 3 && "-p" == StringView(argv[2]))
            p.print(stdout);
    }
    catch (IException& ex)
    {
        ex.printErrorMsg(stderr);
    }
}
