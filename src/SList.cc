#include "adt/logs.hh"
#include "adt/SList.hh"
#include "adt/PoolAllocator.hh"
#include "adt/defer.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    SList<i64> list {};

    PoolAllocator alloc {sizeof(SList<int>::Node), SIZE_1K};
    defer( alloc.freeAll() );

    auto* pOne = list.insert(&alloc, 1);
    auto* pTwo = list.insert(&alloc, 2);
    auto* pThree = list.insert(&alloc, 3);
    auto* pFour = list.insert(&alloc, 4);

    list.remove(&alloc, pOne);
    list.remove(&alloc, pFour);
    list.remove(&alloc, pThree);
    list.remove(&alloc, pTwo);

    list.insert(&alloc, 5);
    list.insert(&alloc, 6);

    CERR("list: {}\n", list);

    list.destroy(&alloc);

    {
        SListManaged<int> l {};
        defer( l.destroy() );
        l.insert(1);
        l.insert(2);
        l.insert(3);

        CERR("l: {}\n", l);
    }
}
