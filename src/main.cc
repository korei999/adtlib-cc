#include "adt/AVLTree.hh"
#include "adt/Arena.hh"
#include "adt/Arr.hh"
#include "adt/Buddy.hh"
#include "adt/FixedAllocator.hh"
#include "adt/FreeList.hh"
#include "adt/List.hh"
#include "adt/Map.hh"
#include "adt/OsAllocator.hh"
#include "adt/Pool.hh"
#include "adt/RBTree.hh"
#include "adt/ThreadPool.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/sort.hh"
#include "json/Parser.hh"

using namespace adt;

/*u8 BIG[SIZE_1G * 4] {};*/

constexpr int total = 1000000;

static void
testRB()
{
    /*ChunkAllocator alloc(sizeof(RBNode<int>), SIZE_1M * 100);*/

    /*FreeList alloc(SIZE_8M);*/
    /*FixedAllocator alloc(BIG, sizeof(BIG));*/
    Arena alloc(SIZE_8M);
    /*OsAllocator alloc;*/

    /*Buddy alloc(SIZE_8M);*/

    /*Arena alloc2(SIZE_8M);*/
    /*defer( ArenaFreeAll(&alloc2) );*/

    defer( alloc.freeAll() );

    RBTree<long> kek(&alloc.super);
    defer( RBDestroy(&kek) );

    Vec<RBNode<long>*> a(&alloc.super, 32);

    bool (*pfnCollect)(RBNode<long>*, RBNode<long>*, void* pArgs) = +[]([[maybe_unused]] RBNode<long>* pPar, RBNode<long>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<RBNode<long>*>*)pArgs;
        a->push(pNode);
        return false;
    };

    [[maybe_unused]] void (*pfnPrintInt)(const RBNode<long>*, void* pArgs) = +[](const RBNode<long>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT("%s" ADT_LOGS_COL_NORM " %d\n", pNode->color == RB_COL::RED ? ADT_LOGS_COL_RED "(R)" : ADT_LOGS_COL_BLUE "(B)", pNode->data);
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

    RBTraverse({}, kek.base.pRoot, pfnCollect, &a, RB_ORDER::PRE);
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
    defer( alloc.freeAll() );

    AVLTree<int> tree(&alloc.super);
    Vec<AVLNode<int>*> a(&alloc.super);

    [[maybe_unused]] void (*pfnPrintInt)(const AVLNode<int>*, void* pArgs) = [](const AVLNode<int>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT(ADT_LOGS_COL_YELLOW "%d" ADT_LOGS_COL_NORM " %d\n", pNode->height, pNode->data);
    };

    [[maybe_unused]] bool (*pfnCollect)(AVLNode<int>*, void* pArgs) = [](AVLNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<AVLNode<int>*>*)pArgs;
        a->push(pNode);
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

    Arena list(SIZE_8K);
    defer( list.freeAll() );

    Vec<int> vec(&list.super);
    int what = 2;

    void* p = list.alloc(what, sizeof(int));
    memset(p, 1, what * sizeof(int));
    for (u32 i = 0; i < 20; ++i)
    {
        vec.push(int(i));

        p = list.realloc(p, what + i, sizeof(int));
    }
}

static void
testLock()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );
    atomic_int number = 1;
    ThreadPool tp(&arena.super);
    tp.start();

    ThreadPoolLock tpLock(INIT);
    defer( tpLock.destroy() );

    auto task = +[](void* pArgs) -> int {
        auto* pNumber = (atomic_int*)pArgs;
        pNumber->fetch_add(1);
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
        auto* pTp = (ThreadPoolLock*)arena.alloc(1, sizeof(ThreadPoolLock));
        pTp->init();

        tp.submitSignal(task, &number, pTp);

        pTp->wait();
        pTp->destroy();
    }

    COUT("after: number: {}\n", (int)number);

    tp.wait();
    COUT("ThreadPoolWait: number: {}\n", (int)number);
    tp.destroy();
}

void
testSort()
{
    Arena arena(SIZE_1M);
    defer( arena.freeAll() );

    srand(1290837027);

    u32 size = 10000000;

    Vec<int> vec(&arena.super);
    vec.setSize(size);

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

    fill();

    t0 = utils::timeNowMS();
    sort::quick<VecBase, int, utils::compareRev<int>>(&vec.base);
    t1 = utils::timeNowMS() - t0;
    assert(sort::sorted(vec.base, sort::DEC));
    COUT("sorted(Rev) {} items in: {} ms\n", size, t1);

    fill();

    t0 = utils::timeNowMS();
    sort::partition(vec.data(), 0, vec.getSize() - 1, vec[vec.getSize() / 2]);
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
    defer( arena.freeAll() );

    VecBase<int> vec;
    vec.push(&arena.super, 1);
    vec.push(&arena.super, 2);
    vec.push(&arena.super, 3);

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
    defer( arena.freeAll() );

    Queue<f64> q(&arena.super);
    q.pushBack(1.0);
    q.pushFront(2.0);
    q.pushBack(99.0);
    q.pushFront(3.0);
    q.pushBack(10.0);
    q.pushFront(-1.0);
    q.pushFront(-20.0);
    q.pushBack(-30.0);

    Vec<int> what(&arena.super);
    what.push(1);
    what.push(3);
    what.push(10);
    what.push(11);
    what.push(-1);

    auto newWhat = what.clone(&arena.super);
    print::out("newWhat: [{}]\n", newWhat);

    print::out("q: {}\n", q);
}

static void
testList()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    List<f64> list(&arena.super);
    auto* pM1 = list.pushFront(-1.0);
    list.pushBack(1.0);
    auto* p2 = list.pushBack(2.0);
    auto* p3 = list.pushBack(3.0);
    auto* p4 = list.pushBack(4.0);
    list.pushFront(5.0);

    {
        auto* pNew = ListNodeAlloc(&arena.super, 999.0);
        list.insertBefore(p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 666.0);
        list.insertAfter(p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 333.0);
        list.insertAfter(p3, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.super, 444.0);
        list.insertBefore(p3, pNew);
    }

    list.remove(p3);
    list.remove(pM1);
    list.remove(p4);

    /*ListDestroy(&list);*/
    List<f64> list0(&arena.super);
    for (auto& el : list) list0.pushBack(el);

    List<f64> list1(&arena.super);
    for (auto& el : list) list1.pushBack(el);

    list0.sort<utils::compareRev>();
    LOG("list0: [{}]\n", list0);
    list1.sort();
    LOG("list1: [{}]\n", list1);
}

constexpr int BIG = 10'000;
static Pool<int, BIG> s_poolOfInts;

void
testPool()
{
    Pool<long, 4> pool(INIT);
    defer( pool.destroy() );

    LOG("sizeof(pool): {}\n", sizeof(pool));

    u32 r0 = pool.getHandle();
    u32 r1 = pool.getHandle();
    u32 r2 = pool.getHandle();
    u32 r3 = pool.getHandle();

    auto& lr1 = pool[r1];
    auto& lr2 = pool[r2];
    lr1 = 5;
    lr2 = 10;

    pool.giveBack(r0);
    pool.giveBack(r3);

    u32 r4 = pool.getHandle();
    u32 r5 = pool.getHandle();

    pool.giveBack(r4);

    auto& lr5 = pool[r5];
    lr5 = 15;

    u32 r6 = pool.getHandle();
    auto& lr6 = pool[r6];
    lr6 = 20;

    LOG("lr1: {}, lr2: {}, lr5: {}, lr6: {}\n", lr1, lr2, lr5, lr6);

    LOG("size: {}\n", pool.nOccupied);

    pool.giveBack(r1);
    pool.giveBack(r2);
    pool.giveBack(r5);
    LOG("size: {}\n", pool.nOccupied);
    pool.giveBack(r6);

    {
        PoolHandle r0 = pool.getHandle();
        auto& lr0 = pool[r0];
        lr0 = 5;

        PoolHandle r1 = pool.getHandle();
        auto& lr1 = pool[r1];
        lr1 = 10;

        PoolHandle r2 = pool.getHandle();
        auto& lr2 = pool[r2];
        lr2 = 15;

        PoolHandle r3 = pool.getHandle();
        auto& lr3 = pool[r3];
        lr3 = 20;

        LOG("lr1: {}, lr2: {}, lr3: {}, lr4: {}\n", lr0, lr1, lr2, lr3);
    }

    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    Vec<PoolNode<int>> vec(&arena.super, BIG);
    vec.setSize(BIG);

    int poolSize = 0;
    int vecSize = 0;

    for (u32 i = 0; i < BIG / 2; i++)
        auto _i = s_poolOfInts.getHandle();

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
    defer( arena.freeAll() );

    ThreadPool tp(&arena.super);
    tp.start();

    atomic_int num = 0;

    auto inc = +[](void* pArg) -> int {
        for (int i = 0; i < 1000; ++i)
            atomic_fetch_add_explicit((atomic_int*)pArg, 1, memory_order_relaxed);

        return 0;
    };

    tp.submit(inc, &num);
    tp.submit(inc, &num);

    tp.wait();
    tp.destroy();

    LOG("num: {}\n", (int)num);
}

struct MethodTester;

struct MethodTesterInterface
{
    void (*what)(MethodTester*);
};

struct MethodTester
{
    MethodTesterInterface* vTable {};

    void what() { vTable->what(this); }
};

struct MethodTesterSub1
{
    /*MethodTesterInterface* vptr {};*/
    MethodTester super {};

    MethodTesterSub1();

    void what() { COUT("MethodTester1: WHAT\n"); }
};

struct MethodTesterSub2
{
    MethodTester super {};

    MethodTesterSub2();

    void what() { COUT("MethodTester2: HELLLO\n"); }
};

inline MethodTesterInterface inl_MethodTester1VTable {
    .what = decltype(MethodTesterInterface::what)(+[](MethodTesterSub1* s) { s->what(); })
};

inline MethodTesterInterface inl_MethodTester2VTable {
    .what = decltype(MethodTesterInterface::what)(+[](MethodTesterSub2* s) { s->what(); })
};

MethodTesterSub1::MethodTesterSub1()
    : super(&inl_MethodTester1VTable) {}

MethodTesterSub2::MethodTesterSub2()
    : super(&inl_MethodTester2VTable) {}

static void
methodWhat(MethodTester* s)
{
    s->what();
}

static void
testMethods()
{
    MethodTesterSub1 mt1;
    MethodTesterSub2 mt2;

    methodWhat((MethodTester*)&mt1);
    methodWhat((MethodTester*)&mt2);
}

static void
testMap()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    Map<String> map(&arena.super);
    map.insert("Hello");
    map.insert("Biden");
    map.insert("it's");
    map.insert("Zelensky");

    auto fHello = map.search("Hello");
    auto fBiden = map.search("Biden");
    auto fIts = map.search("it's");
    auto fZelya = map.search("Zelensky");

    auto fWe = map.search("we");

    assert(*fHello.pData == "Hello");
    assert(*fBiden.pData == "Biden");
    assert(*fIts.pData == "it's");
    assert(*fZelya.pData == "Zelensky");
    assert(fWe.pData == nullptr);

    LOG_GOOD("passed\n");
}

int
main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        COUT("jsonast version: {:.1}\n\n", ADTLIB_CC_VERSION);
        COUT("usage: {} <path to json> [-p(print)|-e(json creation example)]\n", argv[0]);
        return 0;
    }

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

    if (argc >= 2 && (String(argv[1]) == "--method"))
    {
        testMethods();
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
    tp.start();
    defer( alloc.freeAll() );

    if (argc >= 2 && (String(argv[1]) == "--avl" || String(argv[1]) == "--tree"))
    {
        tp.submit([](void*) -> int {
            testAVL();
            return 0;
        }, nullptr);
    }

    if (argc >= 2 && (String(argv[1]) == "--rb" || String(argv[1]) == "--tree"))
    {
        tp.submit([](void*) -> int {
            testRB();
            return 0;
        }, nullptr);
    }

    tp.wait();
    tp.destroy();

    if (String(argv[1]) == "--avl" || String(argv[1]) == "--rb" || String(argv[1]) == "--tree")
        return 0;

    if (argc >= 2)
    {
        Arena arena(SIZE_1M);
        defer( arena.freeAll() );
        /*OsAllocator arena;*/
        /*FreeList arena(SIZE_1G * 2);*/

        json::Parser p(&arena.super);
        json::RESULT res = json::ParserLoadParse(&p, argv[1]);
        LOG("parsed\n");
        if (res == json::FAIL) LOG_WARN("json::ParserLoadAndParse() failed\n");

        if (argc >= 3 && "-p" == String(argv[2]))
            json::ParserPrint(&p, stdout);

        /*json::ParserDestroy(&p);*/
        /*_FreeListPrintTree(&al, &arena.base);*/

    }
    LOG_GOOD("done\n");
}
