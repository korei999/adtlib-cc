#include "adt/logs.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/rng.hh"

#include <algorithm>

using namespace adt;

int
main()
{
    constexpr isize BIG = 2000000;
    rng::PCG32 rng = 666;

    {
        VecM<StringM> v0;
        defer(
            for (auto& e : v0) e.destroy();
            v0.destroy()
        );

        for (isize i = 0; i < BIG; ++i)
        {
            constexpr StringView svChars = "1234567890-=qwertyuiop[]asdfghjklQWERTASDVZXCVKLJ:H";
            char aBuff[33] {};
            const u32 len = rng.nextInRange(16, 32);

            for (isize i = 0; i < len; ++i)
            {
                aBuff[i] = svChars[rng.nextInRange(0, svChars.size() - 1)];
            }

            v0.emplace(aBuff, len);
        }

        {
            auto v1 = v0.clone();
            defer( v1.destroy() );
            {
                const isize t0 = utils::timeNowUS();
                std::sort(v1.data(), v1.data() + v1.size());
                const isize t1 = utils::timeNowUS() - t0;
                LOG_NOTIFY("std::sort(StringM): {} items in {} ms\n", v1.size(), t1 / 1000.0);
            }

            auto v2 = v0.clone();
            defer( v2.destroy() );
            {
                const isize t0 = utils::timeNowUS();
                sort::quick(&v2);
                const isize t1 = utils::timeNowUS() - t0;
                LOG_NOTIFY("sort::quick: {} items in {} ms\n", v2.size(), t1 / 1000.0);
            }

            ADT_ASSERT_ALWAYS(v1.size() == v2.size(), "");
            for (isize i = 0; i < v1.size(); ++i)
                ADT_ASSERT_ALWAYS(v1[i] == v2[i], "");
        }
    }

    CERR("\n");

    {
        VecM<i64> v0 {BIG};
        defer( v0.destroy() );

        v0.setSize(v0.cap());
        for (isize i = 0; i < BIG; ++i)
            v0[i] = isize(rng.next());

        auto v1 = v0.clone();
        defer( v1.destroy() );
        {
            const isize t0 = utils::timeNowUS();
            std::sort(v1.data(), v1.data() + v1.size());
            const isize t1 = utils::timeNowUS() - t0;
            LOG_NOTIFY("std::sort(u32): {} items in {} ms\n", v1.size(), t1 / 1000.0);
        }

        auto v2 = v0.clone();
        defer( v2.destroy() );
        {
            const isize t0 = utils::timeNowUS();
            sort::quick(&v2);
            const isize t1 = utils::timeNowUS() - t0;
            LOG_NOTIFY("sort::quick(u32): {} items in {} ms\n", v2.size(), t1 / 1000.0);
        }

        ADT_ASSERT_ALWAYS(v1.size() == v2.size(), "");
        for (isize i = 0; i < v1.size(); ++i)
            ADT_ASSERT_ALWAYS(v1[i] == v2[i], "");
    }
}
