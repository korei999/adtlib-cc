#include "adt/Arena.hh" /* IWYU pragma: keep */
#include "adt/Array.hh"
#include "adt/BufferAllocator.hh"
#include "adt/List.hh"   /* IWYU pragma: keep */
#include "adt/String.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh"    /* IWYU pragma: keep */
#include "adt/defer.hh"  /* IWYU pragma: keep */
#include "adt/logs.hh"   /* IWYU pragma: keep */
#include "adt/math.hh"   /* IWYU pragma: keep */
#include "adt/time.hh"

#include <format>

using namespace adt;

int
main()
{
    LOG_NOTIFY("print test...\n");

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
        u8 aBuff[32] {};
        BufferAllocator buff {aBuff};

        LOG("realCap: {}\n", buff.realCap());

        String s = print::toString(&buff, buff.realCap(), "\"({}): hello {} {}   \"", 666, "im", "toxic");
        print::out("s({}): '{}'\n", s.size(), s);
        ADT_ASSERT_ALWAYS(s == "\"(666): hello im toxic   \"", "{}", s);
    }

    constexpr isize BIG = 1000000;

    {
        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));
    }

    {
        const auto t0 = time::nowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));

        const auto t1 = time::nowUS();

        // print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(adt::print) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(adt::print) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    CERR("\n");

    {
        const auto t0 = time::nowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            snprintf(aBuff, sizeof(aBuff) - 1, "%lld, %lld, %f, %lf", i, i, f32(i), f64(i));

        const auto t1 = time::nowUS();

        // print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(snprintf) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(snprintf) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    CERR("\n");

    {
        const auto t0 = time::nowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            std::format_to(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));

        const auto t1 = time::nowUS();
        // print::out("aBuff: {}\n", aBuff);

        // LOG_BAD("(std) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(std) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    LOG_GOOD("print test passed\n");
}
