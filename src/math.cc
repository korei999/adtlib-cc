#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#include "WIP/math2.hh"

using namespace adt;

static void
test()
{
    math2::V2 v0;
    math2::IV2 iv0;

    math2::IV2 iv1 = iv0 + iv0;
    math2::V2 v1 = v0.xy();
    math2::IV2 iv2 = static_cast<math2::IV2>(v0);

    math2::V3 v3_0 {1.0f, 2.0f, 3.0f};
    ADT_ASSERT_ALWAYS(v3_0[2] == 3.0f, "{}", v3_0[2]);

    static_assert(sizeof(math2::V2) == 8);
    static_assert(sizeof(math2::IV2) == 8);

    math2::V4 v4_0 {1, 2, 3, 4};
    math2::V3 v3_1 = v4_0.xyz();
    LogDebug{"v4_0: {}, v3_1: {}, what: {}\n", v4_0, v3_1};
}

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer(
        ztp.destroy();
        IThreadPool::setGlobal(nullptr);
    );

    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    test();
}
