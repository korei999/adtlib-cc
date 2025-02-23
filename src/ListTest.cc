#include "adt/logs.hh"
#include "adt/List.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"

using namespace adt;

int
main()
{
    ListManaged<long> list(OsAllocatorGet());
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
    LOG("list: [{}]\n", list);
    list.base.sort();
    LOG("list: [ {:>5} ]\n", list);
}
