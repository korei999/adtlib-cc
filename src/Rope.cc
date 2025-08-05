#include "adt/logs.hh"
#include "adt/Rope.hh"
#include "adt/Arena.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("Rope test...\n");

    {
        Rope r {};
        defer( r.destroy() );

        r.insert("Hello", 0);
        r.insert("_world", 5);
        r.insert("_prepend_", 0);
        r.insert("_2_", 2);
        r.insert("|WHAZZUP|", 8);

        r.printTree(StdAllocator::inst(), stdout);

        print::out("\nfull: '");
        for (isize i = 0; i < r.m_totalSize; ++i)
            print::out("{}", r.charAt(i));
        print::out("'\n");
    }


    LOG_GOOD("Rope test passed\n");
}
