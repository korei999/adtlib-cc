#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#include "WIP/math.hh"

using namespace adt;

static void
test()
{
    {
        math::V2 v0;
        math::IV2 iv0;

        math::IV2 iv1 = iv0 + iv0;
        math::V2 v1 = v0;
        math::IV2 iv2 = static_cast<math::IV2>(v0);

        math::V3 v3_0 {1.0f, 2.0f, 3.0f};
        ADT_ASSERT_ALWAYS(v3_0[0] == 1.0f, "{}", v3_0[0]);
        ADT_ASSERT_ALWAYS(v3_0[1] == 2.0f, "{}", v3_0[1]);
        ADT_ASSERT_ALWAYS(v3_0[2] == 3.0f, "{}", v3_0[2]);

        static_assert(sizeof(math::V2) == 8);
        static_assert(sizeof(math::IV2) == 8);

    }

    {
        math::V4 v4_0 {1, 2, 3, 4};
        v4_0.x() = 99;
        v4_0.y() = 66;
        math::V3 v3_1 = v4_0.xyz();
        ADT_ASSERT_ALWAYS(v4_0 == math::V4(99, 66, 3, 4), "{}", v4_0);
        LogDebug{"v4_0: {}, v3_1: {}\n", v4_0, v3_1};
    }

    {
        math::M2 m2_0 {1};
        LogDebug{"\nm2_0: {:.3}, det: {}, ({}, {})\n", m2_0, m2_0.det(), m2_0[0], m2_0[1]};

        math::M3 m3_0 {1};
        math::M3 m3_1 = m3_0 * 2.0f;
        LogDebug{"\nm3_0: {:.3}\nm3_1: {:.3}\nm3_1.inv(): {:.3}\n", m3_0, m3_1, m3_1.inv()};

        ADT_ASSERT_ALWAYS(m3_1 * m3_1.inv() == math::M3{1}, "{}", m3_1);

        ADT_ASSERT_ALWAYS(m3_0 == m3_0, "");
    }

    {
        math::M4 m4_0 {1};
        math::M4 m4_1 = m4_0.translated({1, 2, 3});
        ADT_ASSERT_ALWAYS(m4_1 * m4_1.inv() == m4_0, "");
        LogDebug{"\nm4_0: {}\nm4_1: {}\nm4_1.inv: {}\n", m4_0, m4_1, m4_1.inv()};
    }

    {
        math::Qt q0 {1};
        LogDebug{"q0: {}\n", q0};
    }
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

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    test();
}
