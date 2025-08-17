#include "adt/logs.hh"
#include "adt/Arena.hh"

#include "Rope.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("Rope test...\n");

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

        r.printTree(StdAllocator::inst(), stdout);

        print::out("\nfull: '");
        for (isize i = 0; i < r.m_totalSize; ++i)
            print::out("{}", r.charAt(i));
        print::out("'\n");

        print::out("\n");
        for (isize i = 0; i < r.size(); ++i)
            print::out("({}){}{}", i, r.charAt(i), i < r.size() - 1 ? ", " : "");
        print::out("\n");

    }


    LOG_GOOD("Rope test passed\n");
}
