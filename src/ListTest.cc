#include "adt/List.hh"
#include "adt/defer.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    ListManaged<long> list {};
    defer( list.destroy() );

    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(5);
    list.pushBack(-123);
    list.pushBack(234);
    list.pushBack(-1);
    list.pushBack(0);

    list.sort<utils::compareRev<long>>();
    print::err("list: {}\n", list);
    list.sort();
    print::err("list: {:>5}\n", list);
}
