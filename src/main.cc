#include "adt/AVLTree.hh"
#include "adt/Arr.hh"
#include "adt/Heap.hh"
#include "adt/List.hh"
#include "adt/Map.hh"
#include "adt/Pool.hh"
#include "adt/RBTree.hh"
#include "adt/ThreadPool.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/sort.hh"
#include "adt/Pair.hh"
#include "adt/math.hh"
#include "adt/Result.hh"
#include "adt/Opt.hh"
#include "adt/String.hh"
#include "adt/guard.hh"

#include "adt/Arena.hh"
#include "adt/Buddy.hh"
#include "adt/MutexArena.hh"
#include "adt/OsAllocator.hh"
#include "adt/FixedAllocator.hh"
#include "adt/FreeList.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/AllocatorPool.hh"

using namespace adt;

/*u8 BIG[SIZE_1G * 4] {};*/

constexpr int total = 100000;

static void
testRB()
{
    /*ChunkAllocator alloc(sizeof(RBNode<int>), SIZE_1M * 100);*/

    FreeList alloc(SIZE_8M);
    LOG_GOOD("sizeof(FreeList::Node): {}\n", sizeof(FreeList::Node));

    /*FixedAllocator alloc(BIG, sizeof(BIG));*/
    /*Arena alloc(SIZE_8M);*/
    /*OsAllocator alloc;*/

    /*Buddy alloc(SIZE_8M);*/

    /*Arena alloc2(SIZE_8M);*/
    /*defer( ArenaFreeAll(&alloc2) );*/

    defer( freeAll(&alloc) );

    RBTree<long> kek(&alloc.super);
    defer( RBDestroy(&kek) );

    Vec<RBNode<long>*> a(&alloc.super, 32);

    bool (*pfnCollect)(RBNode<long>*, void* pArgs) = +[](RBNode<long>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<RBNode<long>*>*)pArgs;
        VecPush(a, pNode);

        return false;
    };

    [[maybe_unused]] void (*pfnPrintInt)(const RBNode<long>*, void* pArgs) = +[](const RBNode<long>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT("%s" ADT_LOGS_COL_NORM " %d\n", pNode->color() == RB_COLOR::RED ? ADT_LOGS_COL_RED "(R)" : ADT_LOGS_COL_BLUE "(B)", pNode->data);
    };

    /*void (*pfnPrintNodes)(const RBNode<FreeListNode>*, void* pArgs) = [](const RBNode<FreeListNode>* pNode, void* pArgs) -> void {*/
    /*    COUT("%s" COL_NORM " %u\n", pNode->color == RB_COL::RED ? COL_RED "(R)" : COL_BLUE "(B)", pNode->data.size);*/
    /*};*/

    f64 t0 = utils::timeNowMS();

    /*RBPrintNodes(&alloc2.base, &alloc.tree, alloc.tree.pRoot, pfnPrintNodes, {}, stdout, {}, false);*/
    /*COUT("\n");*/

    int i = 0;
    for (; i < total; i++)
    {
        long r = rand();
        RBInsert(&kek, r, true);

        /*COUT("inserting '%d'\n", r);*/
    }

    RBTraverse(kek.base.pRoot, pfnCollect, &a, RB_ORDER::PRE);
    auto depth = RBDepth(kek.base.pRoot);

    i = 0;
    for (; i < (int)a.base.size; i += 1)
    {
        RBRemoveAndFree(&kek, a[i]);

        /*RBPrintNodes(&alloc2.base, &alloc.tree, alloc.tree.pRoot, pfnPrintNodes, {}, stdout, {}, false);*/
        /*COUT("\n");*/

        /*RBRemove(&kek, a[i]);*/

        if (i % 2 == 0)
        {
            long r = rand();
            RBInsert(&kek, r, true);
        }
    }

    f64 t1 = utils::timeNowMS();

    /*COUT("alloc depth: %d\n", RBDepth(alloc.tree.pRoot));*/
    /*RBPrintNodes(&alloc2.base, &alloc.tree, alloc.tree.pRoot, pfnPrintNodes, {}, stdout, {}, false);*/
    /*COUT("\n");*/
    /**/
    /*if (kek.pRoot)*/
    /*    RBPrintNodes(&alloc2.base, &kek, kek.pRoot, pfnPrintInt, {}, stdout, {}, false);*/
    /*else COUT("tree is empty\n");*/

    COUT("RB: depth: {}\n", depth);
    COUT("total: {}, size: {}\n", total, a.base.size);
    COUT("time: {:.3} ms\n\n", t1 - t0);
}

static void
testAVL()
{
    Arena alloc(SIZE_8M);

    AVLTree<int> tree(&alloc.super);
    Vec<AVLNode<int>*> a(&alloc.super);

    [[maybe_unused]] void (*pfnPrintInt)(const AVLNode<int>*, void* pArgs) = [](const AVLNode<int>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT(ADT_LOGS_COL_YELLOW "%d" ADT_LOGS_COL_NORM " %d\n", pNode->height, pNode->data);
    };

    [[maybe_unused]] bool (*pfnCollect)(AVLNode<int>*, void* pArgs) = [](AVLNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<AVLNode<int>*>*)pArgs;
        VecPush(a, pNode);
        return false;
    };

    f64 t0 = utils::timeNowMS();

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        AVLInsert(&tree, r, true);
    }

    // AVLTraverse(kek.pRoot, pfnCollect, &a, AVL_ORDER::PRE);
    short depth = AVLDepth(tree.pRoot);

    int i = 0;
    for (; i < (int)a.base.size; i += 1)
    {
        AVLRemove(&tree, a[i]);

        if (i % 2 == 0)
        {
            auto r = rand();
            AVLInsert(&tree, r, true);
        }
    }

    f64 t1 = utils::timeNowMS();

    /*if (kek.pRoot) AVLPrintNodes(&alloc.base, &kek, kek.pRoot, pfnPrintInt, {}, stdout, "", false);*/
    /*else COUT("tree is empty");*/
    /*COUT("\n");*/

    COUT("AVL: depth: {}\n", depth);
    COUT("total: {}, size: {}\n", total, a.base.size);
    COUT("time: {:.3} ms\n\n", t1 - t0);

    ArenaFreeAll(&alloc);
}

static void
testBuddy()
{
    Buddy buddy(128);
    defer( BuddyFreeAll(&buddy) );

    struct BigStruct
    {
        String what {};
    };

    int* pInt = (decltype(pInt))BuddyAlloc(&buddy, 1, sizeof(*pInt));
    long* pLong = (decltype(pLong))BuddyAlloc(&buddy, 1, sizeof(*pLong));
    BigStruct* pBig = (decltype(pBig))BuddyAlloc(&buddy, 1, sizeof(*pBig));

    pBig->what = "What";
    *pLong = 2;
    *pInt = 1;

    pInt = (decltype(pInt))BuddyAlloc(&buddy, 1, sizeof(*pInt));
    *pInt = 4;

    pLong = (decltype(pLong))BuddyAlloc(&buddy, 1, sizeof(*pLong));
    *pLong = 10;

    pBig = (decltype(pBig))BuddyAlloc(&buddy, 1, sizeof(*pBig));

    BuddyFree(&buddy, pInt);
    BuddyFree(&buddy, pLong);
}

static void
testFreeList()
{
    LOG_GOOD("testFreeList()\n");

    Arena arena(SIZE_8K);
    defer( freeAll(&arena) );

    FreeList al(SIZE_1K * 2);
    defer( freeAll(&al) );

    Vec<s64> vec(&al.super);
    int what = 2;

    void* p = alloc(&al, what, sizeof(s64));
    memset(p, 0, what * sizeof(s64));
    for (u32 i = 0; i < 20; ++i)
    {
        VecPush(&vec, s64(i));
        ++what;
        /*LOG("i: {}, p: {}\n", i, p);*/
        p = realloc(&al, p, what, sizeof(s64));
        memset(p, 0, what * sizeof(s64));

        _FreeListVerify(&al);
        _FreeListPrintTree(&al, &arena.super);
        CERR("\n");
    }

    /*COUT("vec:  {}\n", vec);*/
    for (int i = 0; i < what; ++i)
        COUT("{}, ", ((s64*)p)[i]);
    COUT("\n");
}

void
testLock()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );
    atomic_int number = 1;
    ThreadPool tp(&arena.super);
    ThreadPoolStart(&tp);

    ThreadPoolLock tpLock {};
    ThreadPoolLockInit(&tpLock);
    defer( ThreadPoolLockDestroy(&tpLock) );

    auto task = +[](void* pArgs) -> int {
        auto* pNumber = (atomic_int*)pArgs;
        atomic_fetch_add(pNumber, 1);
        return 0;
    };

    [[maybe_unused]] auto task2 = +[](void* pArgs) -> int {
        auto* pNumber = (atomic_int*)pArgs;
        atomic_fetch_add(pNumber, 1);

        return 0;
    };

    COUT("before: number: {}\n", (int)number);

    for (u32 i = 0; i < 10000; ++i)
    {
        auto* pTp = (ThreadPoolLock*)alloc(&arena, 1, sizeof(ThreadPoolLock));
        ThreadPoolLockInit(pTp);

        ThreadPoolSubmitSignal(&tp, task, &number, pTp);

        ThreadPoolLockWait(pTp);
        ThreadPoolLockDestroy(pTp);
    }

    COUT("after: number: {}\n", (int)number);

    ThreadPoolWait(&tp);
    COUT("ThreadPoolWait: number: {}\n", (int)number);

    ThreadPoolDestroy(&tp);
}

void
testSort()
{
    Arena arena(SIZE_1M);
    defer( freeAll(&arena) );

    srand(1290837027);

    u32 size = 1000000;

    Vec<int> vec(&arena.super);
    VecSetSize(&vec, size);

    auto fill = [&] {
        for (u32 i = 0; i < size; ++i)
        {
            auto n = rand() % (size*10);
            if (i & 1) n = -n;
            vec[i] = n;
        }
    };

    fill();

    auto t0 = utils::timeNowMS();
    sort::quick(&vec.base);
    auto t1 = utils::timeNowMS() - t0;
    assert(sort::sorted(vec.base, sort::INC));
    COUT("sorted      {} items in: {} ms\n", size, t1);
    /*COUT("vec: {}\n", vec);*/

    fill();

    t0 = utils::timeNowMS();
    sort::quick<VecBase, int, utils::compareRev>(&vec.base);
    t1 = utils::timeNowMS() - t0;
    assert(sort::sorted(vec.base, sort::DEC));
    COUT("sorted(Rev) {} items in: {} ms\n", size, t1);
    /*COUT("vec: {}\n", vec);*/

    fill();

    t0 = utils::timeNowMS();
    sort::partition(VecData(&vec), 0, VecSize(&vec) - 1, vec[VecSize(&vec) / 2]);
    t1 = utils::timeNowMS() - t0;
    COUT("partition in: {} ms\n", t1);

    std::initializer_list<int> initList {5, 3, 8, 4, 2, 9, 7, 1, 0, 6};
    Arr<int, 12> arr = initList;
    int pivot = arr[2];
    COUT("pivot: {}\n", pivot);
    COUT("arr: [{}]\n", arr);
    auto p = sort::partition(arr.aData, 0, arr.size - 1, pivot);
    COUT("arr: [{}], split: {}({})\n", arr, p, arr[p]);
}

static void
testVec()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );

    VecBase<int> vec;
    VecPush(&vec, &arena.super, 1);
    VecPush(&vec, &arena.super, 2);
    VecPush(&vec, &arena.super, 3);

    COUT("vec: [{}]\n", vec);
}

template<typename ...T>
static void
printThings(const T&... tArgs)
{
    ([&] {
        print::out("{}\n", tArgs);
    } (), ...);
}

static void
testPrint()
{
    int len = 2;
    print::out("{:.{}}\n", len, 123.12345f);

    printThings(1, 2, 3, "four", 5.0f);
}

static void
testQueue()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );

    Queue<f64> q(&arena.super);
    QueuePushBack(&q, 1.0);
    QueuePushFront(&q, 2.0);
    QueuePushBack(&q, 99.0);
    QueuePushFront(&q, 3.0);
    QueuePushBack(&q, 10.0);
    QueuePushFront(&q, -1.0);
    QueuePushFront(&q, -20.0);
    QueuePushBack(&q, -30.0);

    Vec<int> what(&arena.super);
    VecPush(&what, 1);
    VecPush(&what, 3);
    VecPush(&what, 10);
    VecPush(&what, 11);
    VecPush(&what, -1);

    auto newWhat = VecClone(&what, &arena.super);
    print::out("newWhat: [{}]\n", newWhat);

    print::out("q: {}\n", q);
}

static void
testList()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );

    List<f64> list(&arena.super);
    auto* pM1 = ListPushFront(&list, -1.0);
    ListPushBack(&list, 1.0);
    auto* p2 = ListPushBack(&list, 2.0);
    auto* p3 = ListPushBack(&list, 3.0);
    auto* p4 = ListPushBack(&list, 4.0);
    ListPushFront(&list, 5.0);

    {
        auto* pNew = ListNodeAlloc(&arena.super, 999.0);
        ListInsertBefore(&list.base, p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 666.0);
        ListInsertAfter(&list.base, p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 333.0);
        ListInsertAfter(&list.base, p3, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 444.0);
        ListInsertBefore(&list.base, p3, pNew);
    }

    ListRemove(&list, p3);
    ListRemove(&list, pM1);
    ListRemove(&list, p4);

    /*ListDestroy(&list);*/
    List<f64> list0(&arena.super);
    for (auto& el : list) ListPushBack(&list0, el);
    List<f64> list1(&arena.super);
    for (auto& el : list) ListPushBack(&list1, el);

    ListSort<f64, utils::compareRev<f64>>(&list0);
    LOG("list0: [{}]\n", list0);
    ListSort(&list1);
    LOG("list1: [{}]\n", list1);
}

constexpr int BIG = 10'000;
static Pool<int, BIG> s_poolOfInts;

void
testPool()
{
    Pool<long, 4> pool(INIT_FLAG::INIT);
    defer( PoolDestroy(&pool) );

    LOG("sizeof(pool): {}\n", sizeof(pool));

    u32 r0 = PoolRent(&pool);
    u32 r1 = PoolRent(&pool);
    u32 r2 = PoolRent(&pool);
    u32 r3 = PoolRent(&pool);

    auto& lr1 = pool[r1];
    auto& lr2 = pool[r2];
    lr1 = 5;
    lr2 = 10;

    PoolReturn(&pool, r0);
    PoolReturn(&pool, r3);

    u32 r4 = PoolRent(&pool);
    u32 r5 = PoolRent(&pool);

    PoolReturn(&pool, r4);

    auto& lr5 = pool[r5];
    lr5 = 15;

    u32 r6 = PoolRent(&pool);
    auto& lr6 = pool[r6];
    lr6 = 20;

    LOG("lr1: {}, lr2: {}, lr5: {}, lr6: {}\n", lr1, lr2, lr5, lr6);

    LOG("size: {}\n", pool.nOccupied);

    PoolReturn(&pool, r1);
    PoolReturn(&pool, r2);
    PoolReturn(&pool, r5);
    LOG("size: {}\n", pool.nOccupied);
    PoolReturn(&pool, r6);

    {
        PoolHnd r0 = PoolRent(&pool);
        auto& lr0 = pool[r0];
        lr0 = 5;

        PoolHnd r1 = PoolRent(&pool);
        auto& lr1 = pool[r1];
        lr1 = 10;

        PoolHnd r2 = PoolRent(&pool);
        auto& lr2 = pool[r2];
        lr2 = 15;

        PoolHnd r3 = PoolRent(&pool);
        auto& lr3 = pool[r3];
        lr3 = 20;

        LOG("lr1: {}, lr2: {}, lr3: {}, lr4: {}\n", lr0, lr1, lr2, lr3);
    }

    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );
    Vec<PoolNode<int>> vec(&arena.super, BIG);
    VecSetSize(&vec, BIG);

    int poolSize = 0;
    int vecSize = 0;

    for (u32 i = 0; i < BIG / 2; i++)
        [[maybe_unused]] auto _i = PoolRent(&s_poolOfInts);

    f64 t0 = utils::timeNowMS();
    /*for (auto& e : s_poolOfInts)*/
    for (auto it = s_poolOfInts.rbegin(); it != s_poolOfInts.rend(); --it)
        ++poolSize;

    f64 t1 = utils::timeNowMS() - t0;
    LOG("iterated through pool in '{:.3}' ms\n", t1);

    t0 = utils::timeNowMS();

    for (auto& e : vec)
    {
        if (e.bDeleted) continue;

        ++vecSize;
    }

    t1 = utils::timeNowMS() - t0;
    LOG("iterated through vec in '{:.3}' ms\n", t1);

    LOG("vecSize: {}, poolSize: {}\n", vecSize, poolSize);
}

static void
testThreadPool()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );

    ThreadPool tp(&arena.super);
    ThreadPoolStart(&tp);

    atomic_int num = 0;

    auto inc = +[](void* pArg) -> int {
        for (int i = 0; i < 1000; ++i)
            atomic_fetch_add_explicit((atomic_int*)pArg, 1, memory_order_relaxed);

        return 0;
    };

    ThreadPoolSubmit(&tp, inc, &num);
    ThreadPoolSubmit(&tp, inc, &num);

    ThreadPoolWait(&tp);
    ThreadPoolDestroy(&tp);

    LOG("num: {}\n", (int)num);
}

struct CustomKey
{
    u64 number {};

    bool
    operator==(const CustomKey& other)
    {
        return number == other.number;
    }
};

template<>
constexpr u64
hash::func(const CustomKey& k)
{
    return hash::func(k.number);
}

static void
testMap()
{
    Arena arena(SIZE_1M);
    defer( freeAll(&arena) );

    Map<String, int> map(&arena.super);

    using bucket_t = decltype(map.base.aBuckets[0]);
    LOG("sizeof(bucket_t): {}\n", sizeof(bucket_t));

    MapInsert(&map, {"one"}, 1);
    MapInsert(&map, {"two"}, 2);
    MapInsert(&map, {"three"}, 3);
    auto tried = MapTryInsert(&map, {"three"}, 4);
    MapInsert(&map, {"five"}, 5);
    MapInsert(&map, {"six"}, 6);
    MapInsert(&map, {"seven"}, 7);

    MapRemove(&map, String("two"));

    auto one = MapSearch(&map, String("one"));
    auto two = MapSearch(&map, String("two"));
    auto three = MapSearch(&map, String("three"));
    auto four = MapSearch(&map, String("four"));

    assert(one);
    assert(!two);
    assert(three);
    assert(!four);

    LOG("size: {}, capacity: {}\n\n", MapSize(&map), MapCap(&map));

    LOG("one: {}, idx: {}\n", *one.pData, MapIdx(&map, one));
    LOG("two: {} ({})\n", two.pData, two.eStatus);
    LOG("three: {}, idx: {}\n", *three.pData, MapIdx(&map, three));
    LOG("four: {} ({})\n", four.pData, four.eStatus);
    LOG("tried: {} , idx: {}, ({})\n", *tried.pData, MapIdx(&map, tried), tried.eStatus);

    CERR("\n");
    for (auto& [k, v] : map)
        LOG("'{}', {}\n", k, v);
    CERR("\n");

    {
        Map<CustomKey, int> map1(&arena.super);
        MapInsert(&map1, {12}, 1);
        MapInsert(&map1, {99}, 2);
        auto fOne = MapSearch(&map1, {12});
        auto fTwo = MapSearch(&map1, {99});

        assert(fOne);
        assert(fTwo);

        LOG("fOne: ['{}', {}]\n", fOne.pData->key.number, fOne.pData->val);
        LOG("fTwo: ['{}', {}]\n", fTwo.pData->key.number, fTwo.pData->val);
    }

    LOG_GOOD("passed\n");
}

int
main(int argc, char* argv[])
{
    if (argc >= 2 && (String(argv[1]) == "--buddy"))
    {
        testBuddy();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--free"))
    {
        testFreeList();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--lock"))
    {
        testLock();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--sort"))
    {
        testSort();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--print"))
    {
        testPrint();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--queue"))
    {
        testQueue();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--list"))
    {
        testList();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--vec"))
    {
        testVec();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--pool"))
    {
        testPool();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--threadpool"))
    {
        testThreadPool();
        return 0;
    }

    if (argc >= 2 && (String(argv[1]) == "--map"))
    {
        testMap();
        return 0;
    }

    /*FixedAllocator alloc (BIG, size(BIG));*/
    Arena alloc(SIZE_1M * 100);
    ThreadPool tp(&alloc.super, 2);
    ThreadPoolStart(&tp);
    defer( ArenaFreeAll(&alloc) );

    if (argc >= 2 && (String(argv[1]) == "--avl" || String(argv[1]) == "--tree"))
    {
        ThreadPoolSubmit(&tp, [](void*) -> int {
            testAVL();
            return 0;
        }, nullptr);
    }

    if (argc >= 2 && (String(argv[1]) == "--rb" || String(argv[1]) == "--tree"))
    {
        ThreadPoolSubmit(&tp, [](void*) -> int {
            testRB();
            return 0;
        }, nullptr);
    }

    ThreadPoolWait(&tp);
    ThreadPoolDestroy(&tp);

    if (String(argv[1]) == "--avl" || String(argv[1]) == "--rb" || String(argv[1]) == "--tree")
        return 0;

    LOG("done\n");
}
