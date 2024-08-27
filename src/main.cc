#include "adt/AVLTree.hh"
#include "adt/Arena.hh"
#include "adt/ChunkAllocator.hh"
#include "adt/Heap.hh"
#include "adt/RBTree.hh"
#include "adt/ThreadPool.hh"
#include "adt/format.hh"
#include "adt/logs.hh"
#include "json/Parser.hh"

using namespace adt;

/*u8 BIG[SIZE_1G * 4] {};*/

constexpr int total = 1000000;

void
testRB()
{
    Arena alloc2 (SIZE_8M);
    ChunkAllocator alChunk (sizeof(RBNode<int>), SIZE_1M * 100);

    RBTree<int> kek (&alChunk.base);
    Array<RBNode<int>*> a (&alloc2.base);

    bool (*pfnCollect)(RBNode<int>*, RBNode<int>*, void* pArgs) = []([[maybe_unused]] RBNode<int>* pPar, RBNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (Array<RBNode<int>*>*)pArgs;
        ArrayPush(a, pNode);
        return false;
    };

    [[maybe_unused]] void (*pfnPrintInt)(const RBNode<int>*, void* pArgs) = [](const RBNode<int>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT("%s" COL_NORM " %d\n", pNode->color == RB_COL::RED ? COL_RED "(R)" : COL_BLUE "(B)", pNode->data);
    };

    /*void (*pfnPrintNodes)(const RBNode<FreeListNode>*, void* pArgs) = [](const RBNode<FreeListNode>* pNode, void* pArgs) -> void {*/
    /*    COUT("%s" COL_NORM " %u\n", pNode->color == RB_COL::RED ? COL_RED "(R)" : COL_BLUE "(B)", pNode->data.size);*/
    /*};*/

    f64 t0 = utils::timeNowMS();

    /*RBPrintNodes(&alloc2.base, &alloc.tree, alloc.tree.pRoot, pfnPrintNodes, {}, stdout, {}, false);*/
    /*COUT("\n");*/

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        RBInsert(&kek, r, true);

        /*COUT("inserting '%d'\n", r);*/
    }

    RBTraverse({}, kek.pRoot, pfnCollect, &a, RB_ORDER::PRE);
    auto depth = RBDepth(kek.pRoot);

    int i = 0;
    for (; i < (int)a.size; i += 1)
    {
        RBRemoveAndFree(&kek, a[i]);

        /*RBPrintNodes(&alloc2.base, &alloc.tree, alloc.tree.pRoot, pfnPrintNodes, {}, stdout, {}, false);*/
        /*COUT("\n");*/

        /*RBRemove(&kek, a[i]);*/

        if (i % 2 == 0)
        {
            auto r = rand();
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

    COUT("RB: depth: %d\n", depth);
    COUT("total: %d, size: %d\n", total, a.size);
    COUT("time: %lf ms\n\n", t1 - t0);

    RBDestroy(&kek);

    ArenaFreeAll(&alloc2);
    ChunkFreeAll(&alChunk);
}

void
testAVL()
{
    Arena alloc {SIZE_8M};
    /*FreeList alloc(SIZE_1G);*/

    AVLTree<int> kek {&alloc.base};
    Array<AVLNode<int>*> a {&alloc.base};

    [[maybe_unused]] void (*pfnPrintInt)(const AVLNode<int>*, void* pArgs) = [](const AVLNode<int>* pNode, [[maybe_unused]] void* pArgs) -> void {
        COUT(COL_YELLOW "%d" COL_NORM " %d\n", pNode->height, pNode->data);
    };

    bool (*pfnCollect)(AVLNode<int>*, void* pArgs) = [](AVLNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (Array<AVLNode<int>*>*)pArgs;
        ArrayPush(a, pNode);
        return false;
    };

    f64 t0 = utils::timeNowMS();

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        AVLInsert(&kek, r, true);
    }

    AVLTraverse(kek.pRoot, pfnCollect, &a, AVL_ORDER::PRE);
    short depth = AVLDepth(kek.pRoot);

    int i = 0;
    for (; i < (int)a.size; i += 1)
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

    COUT("AVL: depth: %d\n", depth);
    COUT("total: %d, size: %d\n", total, a.size);
    COUT("time: %lf ms\n\n", t1 - t0);

    ArenaFreeAll(&alloc);
}

int
main(int argCount, char* paArgs[])
{
    /*FixedAllocator alloc (BIG, size(BIG));*/
    Arena alloc (SIZE_1M * 100);
    ThreadPool tp (&alloc.base, 2);
    ThreadPoolStart(&tp);

    Array<int> toSort (&alloc.base);
    for (int i = 0; i < 10; i++)
        ArrayPush(&toSort, rand() % 20);

    /*for (auto n : toSort)*/
    /*    COUT("%d, ", n);*/
    /*COUT("\n");*/

    /*auto fnCmp = [](const void* l, const void* r) -> int {*/
    /*    return *(int*)l - *(int*)r;*/
    /*};*/

    for (auto n : toSort)
        COUT("%d, ", n);
    COUT("\n");

    auto st0 = utils::timeNowMS();
    /*qsort(toSort.pData, toSort.size, sizeof(int), fnCmp);*/
    utils::qSort(&toSort);
    /*utils::partition(toSort.pData, 3, toSort.size - 1);*/
    auto st1 = utils::timeNowMS();
    COUT("qSort in %.3lf ms\n", st1 - st0);

    for (auto n : toSort)
        COUT("%d, ", n);
    COUT("\n");

    Heap<int> heap (&alloc.base);
    HeapPushMin(&heap, 3);
    HeapPushMin(&heap, 7);
    HeapPushMin(&heap, -1);
    HeapPushMin(&heap, 5);
    HeapPushMin(&heap, 1);

    for (auto& e : heap.a)
        COUT("%d, ", e);
    COUT("\n");

    [[maybe_unused]] int min = HeapExtractMin(&heap);
    min = HeapExtractMin(&heap);

    for (auto& e : heap.a)
        COUT("%d, ", e);
    COUT("\n");

    char buf[100] {};
    char buf2[40] {};

    auto ten = format::toString(buf, utils::size(buf), -12341230);
    COUT("ten: '%.*s'\n", ten.size, ten.pData);
    auto onePointFive = format::toString(buf2, utils::size(buf2), -1.445990);
    COUT("onePointFive: '%.*s', size: %d\n\n", onePointFive.size, onePointFive.pData, onePointFive.size);

    if (argCount <= 1)
    {
        COUT("jsonast version: %f\n\n", ADTLIB_CC_VERSION);
        COUT("usage: %s <path to json> [-p(print)|-e(json creation example)]\n", paArgs[0]);
        goto cleanup;
    }

    /*srand(round(utils::timeNowMS()));*/

    if (argCount >= 2 && (String(paArgs[1]) == "--avl" || String(paArgs[1]) == "--tree"))
    {
        ThreadPoolSubmit(&tp, [](void*) -> int {
            testAVL();
            return 0;
        }, nullptr);
    }

    if (argCount >= 2 && (String(paArgs[1]) == "--rb" || String(paArgs[1]) == "--tree"))
    {
        ThreadPoolSubmit(&tp, [](void*) -> int {
            testRB();
            return 0;
        }, nullptr);
    }

    ThreadPoolWait(&tp);
    ThreadPoolDestroy(&tp);

    if (String(paArgs[1]) == "--avl" || String(paArgs[1]) == "--rb" || String(paArgs[1]) == "--tree")
    {
        goto cleanup;
    }

    if (argCount >= 2)
    {
        json::Parser p (&alloc.base);
        json::ParserLoad(&p, paArgs[1]);
        json::ParserParse(&p);

        if (argCount >= 3 && "-p" == String(paArgs[2]))
            json::ParserPrint(&p);
    }

cleanup:
    ArenaFreeAll(&alloc);
}
