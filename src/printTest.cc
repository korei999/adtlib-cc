#include "adt/Arena.hh" /* IWYU pragma: keep */
#include "adt/Array.hh"
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/logs.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh" /* IWYU pragma: keep */
#include "adt/List.hh" /* IWYU pragma: keep */
#include "adt/String.hh" /* IWYU pragma: keep */
#include "adt/math.hh" /* IWYU pragma: keep */

#include <format>

using namespace adt;

int
main()
{
    int nSpaces = 2;

    print::out("{:{}}", nSpaces, "");
    print::out("there must be {} spaces before this string\n", nSpaces);

    print::out("'{:>10}'{}\n", "10", 10);

    print::out("'{:{}}'", 10, 1);
    print::out("there must be a single quote before the word 'there'\n");

    print::out("dec: {}, hex: {:#x}, bin: {:#b}\n", 13, 13, 13);

    print::out("precision: {}, float: {}, {:.10}\n", 10, math::PI64, math::PI64);

    print::out("{}\n", 10);

    Array<f64, 32> arr {1.1, 2.2, 3.3, 4.4, 5.5};
    print::out("arr: {:.5}\n", arr);

    Span<f64> spArr(arr.data(), arr.size());
    print::out("spArr: {:.4}\n", spArr);

    {
        print::out("Pair<int, int>(:>3): {:>3}\n", Pair {1, 1});
        print::out("Pair<f32, f32>(:.4): {:.4}\n", Pair {1.1f, 2.2f});
    }

    constexpr isize BIG = 1000000;

    {
        // const auto t0 = utils::timeNowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));

        // const auto t1 = utils::timeNowUS();

        // print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(adt::print) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        // printf("(adt::print) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    CERR("\n");

    {
        const auto t0 = utils::timeNowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            print::toSpan(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));

        const auto t1 = utils::timeNowUS();

        // print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(adt::print) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(adt::print) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    CERR("\n");

    {
        const auto t0 = utils::timeNowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            snprintf(aBuff, sizeof(aBuff) - 1, "%lld, %lld, %f, %lf", i, i, f32(i), f64(i));

        const auto t1 = utils::timeNowUS();

        // print::out("aBuff: {}\n", aBuff);
        // LOG_BAD("(snprintf) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(snprintf) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

    CERR("\n");

    {
        const auto t0 = utils::timeNowUS();

        char aBuff[64] {};
        for (isize i = 0; i < BIG; ++i)
            std::format_to(aBuff, "{}, {}, {}, {}", i, i, f32(i), f64(i));

        const auto t1 = utils::timeNowUS();
        // print::out("aBuff: {}\n", aBuff);

        // LOG_BAD("(std) formatted {} in {} ms\n", BIG, (t1 - t0) / 1000);
        printf("(std) formatted %lld in %lld ms\n", BIG, (t1 - t0) / 1000);
    }

}
