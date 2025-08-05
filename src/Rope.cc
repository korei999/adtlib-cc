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
        /*r.insert("_", 5);*/
        /*r.insert("wol", 6);*/
        /*r.insert("ld", 9);*/
    }


    LOG_GOOD("Rope test passed\n");
}
