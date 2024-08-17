#include "adt/AVL.hh"
#include "adt/ArenaAllocator.hh"
#include "adt/RB.hh"
#include "logs.hh"
#include "json/Parser.hh"
#include "ThreadPool.hh"

#include <math.h>

/*u8 BIG[adt::SIZE_1G * 4] {};*/

constexpr int total = 1000000;

void
testRB()
{
    adt::ArenaAllocator alloc(adt::SIZE_8M);

    adt::RB<int> kek(&alloc.base);
    adt::Array<adt::RBNode<int>*> a(&alloc.base);

    bool (*pfnCollect)(adt::RBNode<int>*, void* pArgs) = [](adt::RBNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (adt::Array<adt::RBNode<int>*>*)pArgs;
        adt::ArrayPush(a, pNode);
        return false;
    };

    /*void (*pfnPrintInt)(const adt::RBNode<int>*, void* pArgs) = [](const adt::RBNode<int>* pNode, void* pArgs) -> void {*/
    /*    COUT("%s" COL_NORM " %d\n", pNode->color == adt::RB_COL::RED ? COL_RED "(R)" : COL_BLUE "(B)", pNode->data);*/
    /*    (*(int*)pArgs)++;*/
    /*};*/

    f64 t0 = adt::timeNowMS();

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        adt::RBInsert(&kek, r);
    }

    adt::RBTraverse(kek.pRoot, pfnCollect, &a, adt::RB_ORDER::PRE);
    auto depth = adt::RBDepth(kek.pRoot);

    int i = 0;
    for (; i < (int)a.size; i += 1)
    {
        adt::RBRemove(&kek, a[i]);

        if (i % 10 == 0)
        {
            auto r = rand();
            adt::RBInsert(&kek, r);
        }
    }

    f64 t1 = adt::timeNowMS();

    /*if (kek.pRoot)*/
    /*    adt::RBPrintNodes(&alloc.base, &kek, kek.pRoot, pfnPrintInt, &count, stdout, {}, false);*/
    /*else COUT("tree is empty\n");*/

    COUT("RB: depth: %d\n", depth);
    COUT("total: %d, size: %d\n", total, a.size);
    COUT("time: %lf ms\n\n", t1 - t0);

    adt::ArenaAllocatorFreeAll(&alloc);
}

void
testAVL()
{
    adt::ArenaAllocator alloc(adt::SIZE_8M);

    adt::AVL<int> kek {&alloc.base};
    adt::Array<adt::AVLNode<int>*> a(&alloc.base);

    /*void (*pfnPrintInt)(const adt::AVLNode<int>*, void* pArgs) = [](const adt::AVLNode<int>* pNode, void* pArgs) -> void {*/
    /*    COUT(COL_YELLOW "%d" COL_NORM " %d\n", pNode->height, pNode->data);*/
    /*    (*(int*)pArgs)++;*/
    /*};*/

    bool (*pfnCollect)(adt::AVLNode<int>*, void* pArgs) = [](adt::AVLNode<int>* pNode, void* pArgs) -> bool {
        auto* a = (adt::Array<adt::AVLNode<int>*>*)pArgs;
        adt::ArrayPush(a, pNode);
        return false;
    };

    f64 t0 = adt::timeNowMS();

    for (int i = 0; i < total; i++)
    {
        auto r = rand();
        adt::AVLInsert(&kek, r);
    }

    adt::AVLTraverse(kek.pRoot, pfnCollect, &a, adt::AVL_ORDER::PRE);
    short depth = adt::AVLDepth(kek.pRoot);

    int i = 0;
    for (; i < (int)a.size; i += 1)
    {
        adt::AVLRemove(&kek, a[i]);

        if (i % 10 == 0)
        {
            auto r = rand();
            adt::AVLInsert(&kek, r);
        }
    }

    f64 t1 = adt::timeNowMS();

    /*if (kek.pRoot) adt::AVLPrintNodes(&alloc.base, &kek, kek.pRoot, pfnPrintInt, &count, stdout, "", false);*/
    /*else COUT("tree is empty");*/
    /*COUT("\n");*/

    COUT("AVL: depth: %d\n", depth);
    COUT("total: %d, size: %d\n", total, a.size);
    COUT("time: %lf ms\n\n", t1 - t0);

    adt::ArenaAllocatorFreeAll(&alloc);
}

int
main(int argCount, char* paArgs[])
{
    /*adt::FixedAllocator alloc(BIG, adt::size(BIG));*/
    adt::ArenaAllocator alloc(adt::SIZE_1M * 100);

    adt::ThreadPool tp(&alloc.base);
    adt::ThreadPoolStart(&tp);

    srand(round(adt::timeNowMS()));

    if (argCount >= 2 && (adt::String(paArgs[1]) == "--avl" || adt::String(paArgs[1]) == "--tree"))
    {
        adt::ThreadPoolSubmit(&tp, [](void*) -> int {
            testAVL();
            return 0;
        }, nullptr);
    }

    if (argCount >= 2 && (adt::String(paArgs[1]) == "--rb" || adt::String(paArgs[1]) == "--tree"))
    {
        adt::ThreadPoolSubmit(&tp, [](void*) -> int {
            testRB();
            return 0;
        }, nullptr);
    }

    adt::ThreadPoolWait(&tp);
    adt::ThreadPoolDestroy(&tp);

    if (adt::String(paArgs[1]) == "--avl" || adt::String(paArgs[1]) == "--rb" || adt::String(paArgs[1]) == "--tree")
        return 0;

    if (argCount <= 1)
    {
        COUT("jsonast version: %f\n\n", ADTLIB_CC_VERSION);
        COUT("usage: %s <path to json> [-p(print)|-e(json creation example)]\n", paArgs[0]);
        return 1;
    }


    if (argCount >= 2)
    {
        json::Parser p(&alloc.base);
        json::ParserLoad(&p, paArgs[1]);
        json::ParserParse(&p);

        if (argCount >= 3 && "-p" == adt::String(paArgs[2]))
            json::ParserPrint(&p);
    }

    adt::ArenaAllocatorFreeAll(&alloc);
}
