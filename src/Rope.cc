#include "adt/ArenaList.hh"
#include "adt/Logger.hh"

#include "WIP/Rope.hh"

using namespace adt;

int
main()
{
    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("Rope test...\n");

    {
        Rope r {};
        defer( r.destroy() );

        /*r.insert("Hello", 0);*/
        /*r.insert("| |", 2);*/
        /*r.insert("/----\\", r.size());*/
        /*r.insert("_WORLD_", r.size());*/
        /*r.insert("<*>", 11);*/

        r.insert("hello", 0);
        r.insert("_", r.size());
        r.insert("wor", r.size());
        r.insert("ld", r.size());
        r.insert("||", 6);

        r.printTree(Gpa::inst(), stdout);

        print::out("\nfull: '");
        for (isize i = 0; i < r.m_totalSize; ++i)
            print::out("{}", r.charAt(i));
        print::out("'\n");

        print::out("\n");
        for (isize i = 0; i < r.size(); ++i)
            print::out("({}){}{}", i, r.charAt(i), i < r.size() - 1 ? ", " : "");
        print::out("\n");

    }


    LogInfo("Rope test passed\n");
}
