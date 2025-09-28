#include "adt/ArenaList.hh" /* IWYU pragma: keep */
#include "adt/Array.hh"
#include "adt/BufferAllocator.hh"
#include "adt/List.hh"   /* IWYU pragma: keep */
#include "adt/String.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh"    /* IWYU pragma: keep */
#include "adt/defer.hh"  /* IWYU pragma: keep */
#include "adt/time.hh"
#include "adt/math.hh"   /* IWYU pragma: keep */
#include "adt/Logger.hh" /* IWYU pragma: keep */
#include "adt/ThreadPool.hh"

#include <format>
#include <tuple>

#if __has_include(<fmt/core.h>)
    #define GOT_FMT
    #define FMT_HEADER_ONLY
    #include <fmt/format.h>
#endif

using namespace adt;

struct Hello
{
    VecM<int> v;
    int i;
    float f;
    Pair<StringView, int> p;
    std::tuple<float, StringView, Array<StringView, 3>, std::tuple<int, int, int>> tup;
};

namespace adt::print
{

template<typename ...TS>
[[maybe_unused]] static isize
format(Context* pCtx, FormatArgs fmtArgs, const std::tuple<TS...>& ts)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;

    return std::apply([pCtx, fmtArgs](const TS&... args) {
        return formatVariadic(pCtx, fmtArgs, args...);
    }, ts);
}

[[maybe_unused]] static isize
format(Context* pCtx, FormatArgs fmtArgs, const Hello& x)
{
    fmtArgs.eFmtFlags |= FormatArgs::FLAGS::PARENTHESES;
    return formatVariadic(pCtx, fmtArgs, x.v, x.i, x.f, x.p, x.tup);
}

} /* namespace adt::print */

int
main()
{
    ThreadPool ztp {SIZE_1G};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("print test...\n");

    {
        Hello h;
        defer( h.v.destroy() );
        h.v.push(1);
        h.v.push(2);
        h.v.push(3);
        h.i = 565;
        h.f = 1245.4123f;
        h.p = {"Hi", 123};
        h.tup = {123.123f, "Hello", {"one", "two", "three"}, {1, 2, 3}};

        print::out("Hello: '{}'\n", h);
    }

    {
        using namespace adt::math;
        M2 m2 = M2Iden();
        M3 m3 = M3Iden();
        M4 m4 = M4Iden() * M4RotXFrom(3*PI32/4);;

        print::out("m2: {:.1}\n", m2);
        print::out("m3: {}\n", m3);
        print::out("m4: {:.3}\n", m4);
    }

    {
        char aBuff[64] {};
        const isize n = print::toSpan(aBuff, "hello", 1);
        ADT_ASSERT_ALWAYS(n == 5, "{}", n);
    }

    constexpr int N_SPACES = 2;

    {
        char aBuff[128] {};
        const isize n = print::toSpan(aBuff, "{:{}}", N_SPACES, "");
        ADT_ASSERT_ALWAYS(StringView("  ") == StringView(aBuff, n), "{}", StringView(aBuff, n));
    }

    {
        char aBuff[128] {};
        const isize n = print::toSpan(aBuff, "'{:>10}'{}", "10", 10);
        ADT_ASSERT_ALWAYS(StringView(aBuff, n) == "'        10'10", "{}", StringView(aBuff, n));
    }

    {
        char aBuff[128] {};
        const isize n = print::toSpan(aBuff, "{:{}}", 10, 1);
        ADT_ASSERT_ALWAYS(StringView(aBuff, n) == "1         ", "{}", StringView(aBuff, n));
    }

    {
        char aBuff[128] {};
        const isize n = print::toSpan(aBuff, "dec: {}, hex: {:#x}, bin: {:#b}, oct: {:#o}", 13, 13, 13, 13);
        ADT_ASSERT_ALWAYS(StringView(aBuff, n) == "dec: 13, hex: 0xd, bin: 0b1101, oct: o15", "{}", StringView(aBuff, n));
    }

    print::out("precision: {}, float: {}, {:.10}\n", 10, math::PI64, math::PI64);
    print::out("{}\n", 10);

    Array<f64, 32> arr {1.1, 2.2, 3.3, 4.4, 5.5};
    print::out("arr: {:.5}\n", arr);

    Span<f64> spArr(arr.data(), arr.size());
    print::out("spArr: {:.4}\n", spArr);

    {
        isize nn = 128;
        f64 nnn = 128.88;

        printf("(05lld): '%05lld', (08lf): '%08lf'\n", nn, nnn);
        print::out("({{:>05}): '{:>05}', ({{:>08}): '{:>08}'\n", nn, nnn);
        print::out("octal({}): '{:#o}'\n", nn, nn);
    }

    {
        print::out("Pair<int, int>(:>3): {:>3}\n", Pair {1, 1});
        print::out("Pair<f32, f32>(:.4): {:.4}\n", Pair {1.1f, 2.2f});
    }

    {
        constexpr isize PREALLOC = 32;
        auto* pStd = StdAllocator::inst();

        constexpr StringView svLong = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        const isize n = print::toFILE<PREALLOC>(pStd, stdout, svLong);
        ADT_ASSERT_ALWAYS(n == svLong.size(), "{}", svLong.size());
        print::out("\npreallocated size: {}, nWritten: {}, svLong: {}\n", PREALLOC, n, svLong.size());
    }

    {
        print::Builder buff {StdAllocator::inst(), 128};
        defer( buff.destroy() );

        buff.print("hello: {}, {}, {}, {}", "hello", 1, 2.2, Pair{"hello", 3.3f});
        StringView s1 = buff.print("|(More Here: {})", "another string");
        print::out("{}\n", s1);
        ADT_ASSERT_ALWAYS(StringView(buff) == "hello: hello, 1, 2.2, (hello, 3.3)|(More Here: another string)", "s: '{}'", StringView(buff));
    }

    {
        u8 aBuff[32] {};
        BufferAllocator buff {aBuff};

        LogDebug("realCap: {}\n", buff.realCap());

        String s = print::toString(&buff, buff.realCap(), "\"({}): hello {} {}   \"", 666, "im", "toxic");
        print::out("s({}): '{}'\n", s.size(), s);
        ADT_ASSERT_ALWAYS(s == "\"(666): hello im toxic   \"", "{}", s);
    }

    constexpr isize BIG = 1000000;
    constexpr StringView svTest = "HHHHHHHHHHHHHHHH";

    {
        char aBuff[128] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "some string here {:5} just taking a bunch of space: {}, {}, {}, {}", svTest, i, i, f32(i), f64(i));
    }

    {
        time::Clock timer {INIT};

        char aBuff[128] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "some string here {:5} just taking a bunch of space: {}, {}, {}, {}", svTest, i, i, f32(i), f64(i));

        const auto t1 = timer.elapsedMSec();

        print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(adt::print) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(adt::print) formatted %lld in %g ms\n", BIG, t1);
    }

    LogDebug("\n");

    {
        time::Clock timer {INIT};

        char aBuff[128] {};
        for (isize i = 0; i < BIG; ++i)
            snprintf(aBuff, sizeof(aBuff) - 1, "some string here %.*s just taking a bunch of space: %lld, %lld, %g, %g", 5, svTest.data(), i, i, f32(i), f64(i));

        const auto t1 = timer.elapsedMSec();

        print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(snprintf) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(snprintf) formatted %lld in %g ms\n", BIG, t1);
    }

    LogDebug("\n");

    {
        time::Clock timer {INIT};

        char aBuff[128] {};
        for (isize i = 0; i < BIG; ++i)
            std::format_to(aBuff, "some string here {:.5} just taking a bunch of space: {}, {}, {}, {}", std::string_view{svTest.data(), svTest.size()}, i, i, f32(i), f64(i));

        const auto t1 = timer.elapsedMSec();

        print::out("aBuff: {}\n", aBuff);
        printf("(std::format_to) formatted %lld in %g ms\n", BIG, t1);
    }

    LogDebug("\n");

#ifdef GOT_FMT
    {
        time::Clock timer {INIT};

        char aBuff[128] {};
        for (isize i = 0; i < BIG; ++i)
            fmt::format_to(aBuff, "some string here {:.5} just taking a bunch of space: {}, {}, {}, {}", std::string_view{svTest.data(), svTest.size()}, i, i, f32(i), f64(i));

        const auto t1 = timer.elapsedMSec();

        print::out("aBuff: {}\n", aBuff);
        printf("(fmt::format_to) formatted %lld in %g ms\n", BIG, t1);
    }
#endif

    LogInfo("print test passed\n");
}
