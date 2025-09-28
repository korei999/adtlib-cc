#include <algorithm>
#include <execution>

#include "adt/sort.hh"
#include "adt/ArenaList.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/time.hh"
#include "adt/rng.hh"
#include "adt/ThreadPool.hh"
#include "adt/Logger.hh"

using namespace adt;

static void
insertion2(void* pArray, isize l, isize h, void* pSpace, isize mSize, isize (*pfnCmp)(const void* l, const void* r))
{
    u8* p = (u8*)pArray;

    for (isize i = l + 1; i < h + 1; ++i)
    {
        memcpy(pSpace, &p[i * mSize], mSize);
        isize j = i;
        for (; j > l && pfnCmp(&p[(j - 1) * mSize], pSpace) > 0; --j)
            memcpy(&p[j * mSize], &p[(j - 1) * mSize], mSize);

        memcpy(&p[j * mSize], pSpace, mSize);
    }
}

static void
quick2(void* pArray, isize l, isize r, void* pSpace, void* pSwap, isize mSize, isize (*pfnCmp)(const void* l, const void* r))
{
    u8* a = (u8*)pArray;

    if (l < r)
    {
        if ((r - l + 1) <= 32)
        {
            insertion2(a, l, r, pSpace, mSize, pfnCmp);
            return;
        }

        memcpy(pSpace, &a[ sort::median3(l, (l + r) / 2, r) * mSize ], mSize);
        isize i = l, j = r;

        while (i <= j)
        {
            while (pfnCmp(&a[i * mSize], pSpace) < 0) ++i;
            while (pfnCmp(&a[j * mSize], pSpace) > 0) --j;

            if (i <= j)
            {
                memcpy(pSwap, &a[i * mSize], mSize);
                memcpy(&a[i * mSize], &a[j * mSize], mSize);
                memcpy(&a[j * mSize], pSwap, mSize);

                ++i, --j;
            }
        }

        quick2(a, l, j, pSpace, pSwap, mSize, pfnCmp);
        quick2(a, i, r, pSpace, pSwap, mSize, pfnCmp);
    }
}

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    constexpr isize BIG = 200000;
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
                time::Clock timer {INIT};
                std::sort(v1.data(), v1.data() + v1.size());
                LogDebug("std::sort(StringM): {} items in {} ms\n", v1.size(), timer.elapsedSec() * 1000.0);
            }

            auto v2 = v0.clone();
            defer( v2.destroy() );
            {
                time::Clock timer {INIT};
                sort::quick(&v2);
                LogDebug("sort::quick(StringM): {} items in {} ms\n", v2.size(), timer.elapsedSec() * 1000.0);
            }

            auto v3 = v0.clone();
            defer( v3.destroy() );
            {
                time::Clock timer {INIT};
                qsort(v3.data(), v3.size(), sizeof(*v3.data()), [](const void* pl, const void* pr) -> int {
                    return utils::compare(*(StringM*)pl, *(StringM*)pr);
                });
                LogDebug("qsort(StringM): {} items in {} ms\n", v3.size(), timer.elapsedSec() * 1000.0);
            }

            auto v4 = v0.clone();
            defer( v4.destroy() );
            {
                time::Clock timer {INIT};
                u8 aBuff0[sizeof(*v4.data())] {};
                u8 aBuff1[sizeof(*v4.data())] {};
                quick2(v4.data(), 0, v4.size() - 1, aBuff0, aBuff1, sizeof(aBuff0), [](const void* pl, const void* pr) {
                    return utils::compare(*(StringM*)pl, *(StringM*)pr);
                });
                LogDebug("quick2(StringM): {} items in {} ms\n", v4.size(), timer.elapsedSec() * 1000.0);
            }

            ADT_ASSERT_ALWAYS(v1.size() == v2.size(), "");
            for (isize i = 0; i < v1.size(); ++i)
                ADT_ASSERT_ALWAYS(v1[i] == v2[i], "(i: {}): {}, {}", i, v1[i], v2[i]);

            for (isize i = 0; i < v3.size(); ++i)
                ADT_ASSERT_ALWAYS(v3[i] == v4[i], "(i: {}): {}, {}", i, v3[i], v4[i]);
        }
    }

    print::err("\n");

    {
        constexpr isize QSIZE = 2048;
        static_assert(isPowerOf2(QSIZE));
        ThreadPool tp {QSIZE, SIZE_8G};
        defer( tp.destroy() );

        VecM<i64> v0 {BIG * 100};
        defer( v0.destroy() );

        v0.setSize(v0.cap());
        for (isize i = 0; i < v0.size(); ++i)
            v0[i] = isize(rng.next());

        auto v1 = v0.clone();
        defer( v1.destroy() );
        {
            time::Clock timer {INIT};
#if defined __APPLE__ || defined __OpenBSD__
            std::sort(v1.data(), v1.data() + v1.size());
#else
            std::sort(std::execution::par, v1.data(), v1.data() + v1.size());
#endif
            LogDebug("std::sort(i64)(std::execution::par): {} items in {} ms\n", v1.size(), timer.elapsedSec() * 1000.0);
        }

        auto v2 = v0.clone();
        defer( v2.destroy() );
        {
            time::Clock timer {INIT};
            sort::quickParallel(&tp, &v2);
            LogDebug("sort::quickParallel(i64): {} items in {} ms\n", v2.size(), timer.elapsedSec() * 1000.0);
        }

        ADT_ASSERT_ALWAYS(v1.size() == v2.size(), "");
        for (isize i = 0; i < v1.size(); ++i)
            ADT_ASSERT_ALWAYS(v1[i] == v2[i], "(i: {}): {}, {}", i, v1[i], v2[i]);
    }

    print::err("\n");

    {
        VecM<i64> v0 {BIG};
        defer( v0.destroy() );

        v0.setSize(v0.cap());
        for (isize i = 0; i < BIG; ++i)
            v0[i] = isize(rng.next());

        auto v1 = v0.clone();
        defer( v1.destroy() );
        {
            time::Clock timer {INIT};
            std::sort(v1.data(), v1.data() + v1.size());
            LogDebug("std::sort(u32): {} items in {} ms\n", v1.size(), timer.elapsedSec() * 1000.0);
        }

        auto v2 = v0.clone();
        defer( v2.destroy() );
        {
            time::Clock timer {INIT};
            sort::quick(&v2);
            LogDebug("sort::quick(u32): {} items in {} ms\n", v2.size(), timer.elapsedSec() * 1000.0);
        }

        ADT_ASSERT_ALWAYS(v1.size() == v2.size(), "");
        for (isize i = 0; i < v1.size(); ++i)
            ADT_ASSERT_ALWAYS(v1[i] == v2[i], "(i: {}): {}, {}", i, v1[i], v2[i]);
    }
}
