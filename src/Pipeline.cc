#include "adt/Pipeline.hh"
#include "adt/Vec.hh"
#include "adt/Logger.hh"

using namespace adt;

static void
stage0(void* pArg)
{
    int& arg = *static_cast<int*>(pArg);
    arg = 0;
}

static void
stage1(void* pArg)
{
    int& arg = *static_cast<int*>(pArg);
    arg += 1;
}

static void
stage2(void* pArg)
{
    int& arg = *static_cast<int*>(pArg);
    arg *= 2;
}

static void
stage3(void* pArg)
{
    int& arg = *static_cast<int*>(pArg);
    arg *= 4;
}

static void
stage4(void* pArg)
{
    int& arg = *static_cast<int*>(pArg);
    arg *= 8;
}

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("Pipeline test...\n");

    Gpa* pAl = Gpa::inst();

    Pipeline pl {pAl, {
        {pAl, stage0}, {pAl, stage1}, {pAl, stage2}, {pAl, stage3}, {pAl, stage4}
    }};

    constexpr isize SIZE = 100;

    VecM<int> v {SIZE};
    defer( v.destroy() );

    for (isize i = 0; i < SIZE; ++i)
        v.push(0);

    for (int& e : v)
        pl.add(&e);

    pl.destroy(pAl);

    for (const int e : v)
        ADT_ASSERT_ALWAYS(e == 64, "e: {}", e);

    LogInfo("Pipeline test passed\n");
}
