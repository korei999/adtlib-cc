#include "adt/logs.hh"
#include "adt/List.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    ChunkAllocator chunks(sizeof(ListNode<long>), SIZE_1K);
    defer( chunks.freeAll() );

    List<long> list(&chunks.super);

    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(5);
    list.pushBack(-123);
    list.pushBack(234);
    list.pushBack(-1);
    list.pushBack(0);

    list.sort<utils::compareRev<long>>();
    LOG("list: {}\n", list);
    list.base.sort();
    LOG("list: {}\n", list);
}
