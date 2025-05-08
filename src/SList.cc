#include "adt/logs.hh"
#include "adt/SList.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    SList<i64> list {};

    ChunkAllocator alloc {sizeof(SList<int>::Node), SIZE_1K};
    defer( alloc.freeAll() );

    auto* pOne = list.insert(&alloc, 1);
    auto* pTwo = list.insert(&alloc, 2);
    auto* pThree = list.insert(&alloc, 3);
    auto* pFour = list.insert(&alloc, 4);

    list.removeFree(&alloc, pOne);
    list.removeFree(&alloc, pFour);
    list.removeFree(&alloc, pThree);
    list.removeFree(&alloc, pTwo);

    list.insert(&alloc, 5);
    list.insert(&alloc, 6);

    for (auto& e : list)
    {
        CERR("e: {}\n", e);
    }
}
