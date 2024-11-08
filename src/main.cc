#include "adt/AVLTree.hh"
#include "adt/Arena.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/RBTree.hh"
#include "adt/ThreadPool.hh"
#include "adt/logs.hh"
#include "json/Parser.hh"
#include "adt/defer.hh"
#include "adt/Buddy.hh"
#include "adt/FreeList.hh"
#include "adt/OsAllocator.hh"

#include <stdatomic.h>

using namespace adt;

/*u8 BIG[SIZE_1G * 4] {};*/

constexpr int total = 1000000;

static void
testRB()
{
    /*ChunkAllocator alloc(sizeof(RBNode<int>), SIZE_1M * 100);*/
    FreeList alloc(SIZE_8M);
    /*OsAllocator alloc;*/

    /*Buddy alloc(SIZE_8M);*/

    Arena alloc2(SIZE_8M);
    defer( ArenaFreeAll(&alloc2) );
    /*defer( freeAll(&alloc) );*/

    RBTree<long> kek(&alloc.base);
    defer( RBDestroy(&kek) );

    Vec<RBNode<long>*> a(&alloc2.base, 32);

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

    for (int i = 0; i < total; i++)
    {
        long r = rand();
        RBInsert(&kek, r, true);

        /*COUT("inserting '%d'\n", r);*/
    }

    RBTraverse({}, kek.base.pRoot, pfnCollect, &a, RB_ORDER::PRE);
    auto depth = RBDepth(kek.base.pRoot);

    int i = 0;
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

    FreeList list(SIZE_8K);
    defer( FreeListFreeAll(&list) );

    Vec<int> vec(&list.base);
    int what = 2;

    void* p = alloc(&list, what, sizeof(int));
    memset(p, 1, what * sizeof(int));
    for (u32 i = 0; i < 20; ++i)
    {
        VecPush(&vec, int(i));
        /*COUT("vec: {}\n", vec.base);*/

        /*free(&list, p);*/
        /*p = alloc(&list, what + i, sizeof(int));*/

        p = realloc(&list, p, what + i, sizeof(int));
    }


    /*_FreeListPrintTree(&list);*/
    /*int* p0 = (int*)alloc(&list, 2, sizeof(int));*/
    /*free(&list, p0);*/
    /**/
    /*p0 = (int*)alloc(&list, 2, sizeof(int));*/
    /**/
    /*_FreeListPrintTree(&list);*/
    /**/
    /*p0 = (int*)realloc(&list, p0, 4, sizeof(int));*/
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
        utils::sleepMS(1000.0);
        atomic_fetch_add(pNumber, 1);
        return 0;
    };

    auto task2 = +[](void* pArgs) -> int {
        auto* pNumber = (atomic_int*)pArgs;
        utils::sleepMS(1500.0);
        atomic_fetch_add(pNumber, 1);

        return 0;
    };

    ThreadPoolSubmitLocked(&tp, task, &number, &tpLock);
    ThreadPoolSubmit(&tp, task2, &number);
    ThreadPoolSubmit(&tp, task2, &number);

    COUT("before: number: {}\n", (int)number);
    ThreadPoolLockWait(&tpLock);
    COUT("ThreadPoolLockWait: number: {}\n", (int)number);

    ThreadPoolWait(&tp);
    COUT("ThreadPoolWait: number: {}\n", (int)number);

    ThreadPoolDestroy(&tp);
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

        json::Parser p(&arena.base);
        json::ParserLoadAndParse(&p, argv[1]);
        /*defer(  json::ParserDestroy(&p) );*/

        if (argc >= 3 && "-p" == String(argv[2]))
            json::ParserPrint(&p, stdout);
    }
}
