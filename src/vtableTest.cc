#include "adt/logs.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"
#include "adt/RBTree.hh"

using namespace adt;

struct Big
{
    long l0;
    long l1;
    long l2;
    long l3;

    constexpr s64
    operator-(const Big& r) const
    {
        return (l0 + l1 + l2 + l3) - (r.l0 + r.l1 + r.l2 + r.l3);
    }
};

/* old vtable using stabs */
inline const AllocatorVTable inl_FreeListVTableV1 {
    .alloc = decltype(AllocatorVTable::alloc)(+[](FreeList* s, u64 mCount, u64 mSize) { return s->alloc(mCount, mSize); }),
    .zalloc = decltype(AllocatorVTable::zalloc)(+[](FreeList* s, u64 mCount, u64 mSize) { return s->zalloc(mCount, mSize); }),
    .realloc = decltype(AllocatorVTable::realloc)(+[](FreeList* s, void* ptr, u64 mCount, u64 mSize) { return s->realloc(ptr, mCount, mSize); }),
    .free = decltype(AllocatorVTable::free)(+[](FreeList* s, void* ptr) { return s->free(ptr); }),
    .freeAll = decltype(AllocatorVTable::freeAll)(+[](FreeList* s) { return s->freeAll(); }),
};

int
main()
{
    FreeList fl(SIZE_8M);
    defer( fl.freeAll() );

    {
        RBTree<Big> tree(&fl.super);

        constexpr long max = 1000000;
        for (long i = 0; i < max; ++i)
            tree.insert({i + 0, i + 1, i + 2, i + 3}, true);

        tree.destroy();

        LOG("warmup...\n");
    }

    {
        fl.super = {&inl_FreeListVTableV1};

        RBTree<Big> tree(&fl.super);

        auto t0 = utils::timeNowMS();
        constexpr long max = 1000000;
        for (long i = 0; i < max; ++i)
            tree.insert({i + 0, i + 1, i + 2, i + 3}, true);

        tree.destroy();
        auto t1 = utils::timeNowMS();

        LOG("vtableV1: {} insertions and deletions in {} ms\n", max, t1 - t0);
    }
    {
        /* new vtable using hacks */
        fl.super = {&inl_FreeListVTable};

        RBTree<Big> tree(&fl.super);

        auto t0 = utils::timeNowMS();
        constexpr long max = 1000000;
        for (long i = 0; i < max; ++i)
            tree.insert({i + 0, i + 1, i + 2, i + 3}, true);

        tree.destroy();
        auto t1 = utils::timeNowMS();

        LOG("vtableV2: {} insertions and deletions in {} ms\n", max, t1 - t0);
    }
}
