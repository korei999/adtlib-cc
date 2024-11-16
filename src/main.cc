#include "adt/AVLTree.hh"
#include "adt/Arena.hh"
#include "adt/Arr.hh"
#include "adt/Buddy.hh"
#include "adt/FreeList.hh"
#include "adt/List.hh"
#include "adt/RBTree.hh"
#include "adt/ThreadPool.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/sort.hh"
#include "json/Parser.hh"
#include "adt/Arena2.hh"
#include "adt/FixedAllocator.hh"

using namespace adt;

u8 BIG[SIZE_1G * 4] {};

constexpr int total = 1000000;

static void
testRB()
{
    /*ChunkAllocator alloc(sizeof(RBNode<int>), SIZE_1M * 100);*/

    /*FreeList alloc(SIZE_8M);*/
    /*FixedAllocator alloc(BIG, sizeof(BIG));*/
    Arena2 alloc(SIZE_8M);
    /*OsAllocator alloc;*/

    /*Buddy alloc(SIZE_8M);*/

    /*Arena alloc2(SIZE_8M);*/
    /*defer( ArenaFreeAll(&alloc2) );*/
    /*defer( freeAll(&alloc) );*/

    RBTree<long> kek(&alloc.base);
    defer( RBDestroy(&kek) );

    Vec<RBNode<long>*> a(&alloc.base, 32);

    bool (*pfnCollect)(RBNode<long>*, RBNode<long>*, void* pArgs) = +[]([[maybe_unused]] RBNode<long>* pPar, RBNode<long>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<RBNode<long>*>*)pArgs;
        VecPush(a, pNode);
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
    Arena alloc {SIZE_8M};

    AVLTree<int> kek {&alloc.base};
    Vec<AVLNode<int>*> a {&alloc.base};

    [[maybe_unused]] void (*pfnPrintInt)(const AVLNode<int>*, void* pArgs) = [](const AVLNode<int>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT(ADT_LOGS_COL_YELLOW "%d" ADT_LOGS_COL_NORM " %d\n", pNode->height, pNode->data);
    };

    bool (*pfnCollect)(AVLNode<int>*, void* pArgs) = [](AVLNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (Vec<AVLNode<int>*>*)pArgs;
        VecPush(a, pNode);
        return false;
    };

    f64 t0 = utils::timeNowMS();

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        AVLInsert(&kek, r, true);
    }

    // AVLTraverse(kek.pRoot, pfnCollect, &a, AVL_ORDER::PRE);
    short depth = AVLDepth(kek.pRoot);

    int i = 0;
    for (; i < (int)a.base.size; i += 1)
    {
        AVLRemove(&kek, a[i]);

        if (i % 2 == 0)
        {
            auto r = rand();
            AVLInsert(&kek, r, true);
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
    /*pBig->what = "kekw";*/

    BuddyFree(&buddy, pInt);
    BuddyFree(&buddy, pLong);
}

static void
testFreeList()
{
    LOG_GOOD("testFreeList()\n");

    Arena2 list(SIZE_8K);
    defer( freeAll(&list) );

    Vec<int> vec(&list.base);
    int what = 2;

    void* p = alloc(&list, what, sizeof(int));
    memset(p, 1, what * sizeof(int));
    for (u32 i = 0; i < 20; ++i)
    {
        VecPush(&vec, int(i));

        p = realloc(&list, p, what + i, sizeof(int));
    }
}

void
testLock()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );
    atomic_int number = 1;
    ThreadPool tp(&arena.base);
    ThreadPoolStart(&tp);

    ThreadPoolLock tpLock {};
    ThreadPoolLockInit(&tpLock);
    defer( ThreadPoolLockDestroy(&tpLock) );

    auto task = +[](void* pArgs) -> int {
        auto* pNumber = (atomic_int*)pArgs;
        atomic_fetch_add(pNumber, 1);
        return 0;
    };

    auto task2 = +[](void* pArgs) -> int {
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
    Arena2 arena(SIZE_1K);
    defer( freeAll(&arena) );

    srand(1290837027);

    u32 size = 70;

    Vec<int> vec(&arena.base);
    for (u32 i = 0; i < size; ++i)
        VecPush(&vec, rand() % 100);

    sort::quick(&vec.base);
    /*COUT("inc: {}\n", vec);*/
    assert(sort::sorted(vec.base, sort::INC));

    for (u32 i = 0; i < size; ++i)
        vec[i] = rand() % 100;
    sort::quick<VecBase, int, [](const int& l, const int& r) -> s64 { return r - l; }>(&vec.base);
    /*COUT("dec: {}\n", vec);*/
    assert(sort::sorted(vec.base, sort::DEC));
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
    Arena2 arena(SIZE_1K);
    defer( freeAll(&arena) );

    Queue<f64> q(&arena.base);
    QueuePushBack(&q, 1.0);
    QueuePushFront(&q, 2.0);
    QueuePushBack(&q, 99.0);
    QueuePushFront(&q, 3.0);
    QueuePushBack(&q, 10.0);
    QueuePushFront(&q, -1.0);
    QueuePushFront(&q, -20.0);
    QueuePushBack(&q, -30.0);

    print::out("q: {}\n", q);
}

static void
testList()
{
    Arena arena(SIZE_1K);
    defer( freeAll(&arena) );

    List<f64> list(&arena.base);
    auto* pM1 = ListPushFront(&list, -1.0);
    ListPushBack(&list, 1.0);
    auto* p2 = ListPushBack(&list, 2.0);
    auto* p3 = ListPushBack(&list, 3.0);
    auto* p4 = ListPushBack(&list, 4.0);
    ListPushFront(&list, 5.0);

    {
        auto* pNew = ListNodeAlloc(&arena.base, 999.0);
        ListInsertBefore(&list.base, p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.base, 666.0);
        ListInsertAfter(&list.base, p2, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.base, 333.0);
        ListInsertAfter(&list.base, p3, pNew);
    }
    {
        auto* pNew = ListNodeAlloc(&arena.base, 444.0);
        ListInsertBefore(&list.base, p3, pNew);
    }

    ListRemove(&list, p3);
    ListRemove(&list, pM1);
    ListRemove(&list, p4);

    /*ListDestroy(&list);*/
    List<f64> list0(&arena.base);
    for (auto& el : list) ListPushBack(&list0, el);
    List<f64> list1(&arena.base);
    for (auto& el : list) ListPushBack(&list1, el);

    ListSort<f64, utils::compareRev<f64>>(&list0);
    LOG("list0: [{}]\n", list0);
    ListSort(&list1);
    LOG("list1: [{}]\n", list1);

    Arr<int, 36> arr;
    ArrPush(&arr, 1);
    ArrPush(&arr, 5);
    ArrPush(&arr, 4);
    ArrPush(&arr, 3);
    ArrPush(&arr, -1);
    sort::quick(&arr);
    LOG("arr: [{}]\n", arr);
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

    /*FixedAllocator alloc (BIG, size(BIG));*/
    Arena alloc(SIZE_1M * 100);
    ThreadPool tp(&alloc.base, 2);
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

    if (argc >= 2)
    {
        Arena arena(SIZE_1M);
        defer( freeAll(&arena) );
        /*OsAllocator arena;*/
        /*FreeList al(SIZE_1G * 2);*/

        json::Parser p(&arena.base);
        json::ParserLoadAndParse(&p, argv[1]);

        if (argc >= 3 && "-p" == String(argv[2]))
            json::ParserPrint(&p, stdout);

        /*json::ParserDestroy(&p);*/
        /*_FreeListPrintTree(&al, &arena.base);*/
    }
}
