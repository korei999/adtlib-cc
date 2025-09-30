#include "adt/ArenaList.hh"
#include "adt/BufferAllocator.hh" /* IWYU pragma: keep */
#include "adt/Gpa.hh" /* IWYU pragma: keep */
#include "adt/Queue.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/file.hh"
#include "json/Parser.hh"
#include "adt/Arena.hh"
#include "adt/Logger.hh"

// #include "adt/MiMalloc.hh" /* IWYU pragma: keep */

using namespace adt;

// [[maybe_unused]] static u8 s_aMem[SIZE_1G] {};

int
main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        print::out("usage: {} <path to json> [-p(print)|-e(json creation example)]\n", argv[0]);
        return 0;
    }

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    if (argc >= 1 && StringView(argv[1]) == "-e")
    {
        ArenaList al(SIZE_1K);
        defer( al.freeAll() );

        auto jObj = json::makeObject(&al, ""); /* root object usually has no name, this json parser allows to have it */
        jObj.pushToObject(&al, json::makeNumber("fifteen", 15));
        jObj.pushToObject(&al, json::makeFloat("fifteen.dot.fifteen", 15.15));
        u32 arrIdx = jObj.pushToObject(&al, json::makeArray(&al, "arr"));
        /* array values have no keys, but this json parser makes no distiction between arrays and objects, so the keys are empty here */
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string0"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string1"));
        jObj[arrIdx].pushToArray(&al, json::makeString("", "string2"));

        json::printNode(Gpa::inst(), stdout, &jObj);
        putchar('\n');

        return 0;
    }

    try
    {
        // FreeList al {SIZE_1G};
        // Gpa al;
        // MiMalloc al;
        Arena al {SIZE_8G};
        // MiHeap al(0);
        defer(
            // LOG_GOOD("arena: totalBytes: {}\n", al.nBytesOccupied());
            al.freeAll()
        );

        String sJson = file::load(&al, argv[1]);

        if (!sJson) return 1;

        // defer( sJson.destroy(&al) );

        ADT_ALLOCA(json::Parser, p);
        bool eRes = p.parse(&al, sJson);
        if (!eRes) LogWarn("json::Parser::parse() failed\n");
        // defer( p.destroy() );

        if (argc >= 3 && "-p" == StringView(argv[2]))
            p.print(Gpa::inst(), stdout);
    }
    catch (const std::exception& ex)
    {
        print::err(ex.what());
    }
}
