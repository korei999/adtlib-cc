#include "adt/Arena.hh"
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/FreeList.hh" /* IWYU pragma: keep */
#include "adt/MutexArena.hh" /* IWYU pragma: keep */
#include "adt/OsAllocator.hh" /* IWYU pragma: keep */
#include "adt/Queue.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/file.hh"
#include "adt/logs.hh"
#include "json/Parser.hh"

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

    if (argc >= 1 && String(argv[1]) == "-e")
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
        /*FixedAllocator al(s_aMem);*/
        Arena al(SIZE_8M);
        /*FreeList al(SIZE_1G);*/
        /*OsAllocator al;*/
        defer( al.freeAll() );

        Opt<String> o_sJson = file::load(&al, argv[1]);

        if (!o_sJson) return 1;
        /*defer( o_sJson.value().destroy(OsAllocatorGet()) );*/

        json::Parser p {};
        json::STATUS eRes = p.parse(&al, o_sJson.value());
        if (eRes == json::STATUS::FAIL) LOG_WARN("json::Parser::parse() failed\n");
        /*defer( p.destroy() );*/

        if (argc >= 3 && "-p" == String(argv[2]))
            p.print(stdout);

        /*_FreeListPrintTree(&al, &arena.base);*/
    }
    catch (IException& ex)
    {
        ex.logErrorMsg();
    }
}
