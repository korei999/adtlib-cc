#include "adt/ArenaList.hh"
#include "adt/Vec.hh"
#include "adt/defer.hh"
#include "adt/ReverseIt.hh"
#include "adt/rng.hh"
#include "adt/Thread.hh"
#include "adt/sort.hh"
#include "adt/utf8.hh"
#include "adt/Logger.hh"

#include <clocale>
#include <string>
#include <vector>

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    {
        constexpr StringView sv0 = "sv1";
        constexpr StringView sv1 = "sv2";
        isize what = utils::Comparator<StringView>{}(sv0, sv1);
        ADT_ASSERT_ALWAYS(what < 0, "{}", what);
    }

    setlocale(LC_ALL, "");

    ArenaList arena(SIZE_1K);
    defer( arena.freeAll() );

    String sHello = String(&arena, "What is this");

    bool bNull = sHello.contains("");

    bool bWhat = sHello.contains("What");
    bool bIs = sHello.contains("is");
    bool bThis = sHello.contains("this");

    bool bWh = sHello.contains("Wh");
    bool bHis = sHello.contains("his");

    bool bHWha = sHello.contains("HWha");

    LogDebug("null: {}, What: {}, is: {}, this: {}, Wh: {}, his: {}, HWha: {}\n", bNull, bWhat, bIs, bThis, bWh, bHis, bHWha);

    sHello.destroy(&arena);

    {
        String sTest = String(&arena, "Test Of This String");
        ADT_ASSERT_ALWAYS(sTest.contains("Test"), "");
        ADT_ASSERT_ALWAYS(sTest.contains("Of"), "");
        ADT_ASSERT_ALWAYS(sTest.contains("This"), "");
        ADT_ASSERT_ALWAYS(sTest.contains("String"), "");
        ADT_ASSERT_ALWAYS(sTest.contains("Strin"), "");
        ADT_ASSERT_ALWAYS(!sTest.contains("Strig"), "");
    }

    {
        StringView s0 = "HELLO BIDEN";
        for (auto ch : ReverseIt(s0))
            putchar(ch);
        putchar('\n');
    }

    {
        const StringView s = "私は日本語を話せません";
        ADT_ASSERT(s.endsWith("せません"), "");
        print::out("s: '{}'\n", s);

        print::out("\"");
        for (auto wc : StringWCharIt(s))
            print::out("'{}'", wc);
        print::out("\"\n");
    }

    {
        constexpr StringView s = "word0 word1 word2 word3";
        constexpr StringView aWords[] {
            "word0", "word1", "word2", "word3",
        };
        int i = 0;
        for (const auto& svWord : StringWordIt(s))
        {
            ADT_ASSERT_ALWAYS(aWords[i] == svWord, "exp: '{}', got: '{}'", aWords[i], svWord);
            ++i;
        }
    }

    {
        constexpr StringView s = "comma0,comma1,comma2,comma3";
        constexpr StringView aWords[] {
            "comma0", "comma1", "comma2", "comma3"
        };
        int i = 0;
        for (const auto& svWord : StringWordIt(s, ","))
        {
            ADT_ASSERT_ALWAYS(aWords[i] == svWord, "exp: '{}', got: '{}'", aWords[i], svWord);
            ++i;
        }
    }

    {
        constexpr StringView s = "STR0 | STR1 | STR2";
        constexpr StringView aWords[] {
            "STR0", "STR1", "STR2"
        };
        int i = 0;
        for (const auto& svWord : StringWordIt(s, " |"))
        {
            ADT_ASSERT_ALWAYS(aWords[i] == svWord, "exp: '{}', got: '{}'", aWords[i], svWord);
            ++i;
        }
    }

    {
        constexpr StringView sv = "Z̤͔ͧ̑̓ä͖̭̈̇lͮ̒ͫǧ̗͚̚o̙̔ͮ̇͐̇";
        int i = 0;
        print::out("{}\n", sv);
        for (const wchar_t& g : StringWCharIt(sv))
            print::out("({}): '{}' ({}), width: {}\n", i++, g, u32(g), wcWidth(g));
    }

    print::out("\n");

    {
        constexpr StringView sv = "| 🇺🇦КВИТКИ/̶̢̧̠̩̠̠̪̜͚͙̏͗̏̇̑̈͛͘ͅ";
        print::out("width of '{}': {}\n", "🇺🇦", utf8::computeWidth("🇺🇦"));
        int i = 0;
        print::out("({}) glyphs...\n", sv);
        for (const wchar_t& g : StringWCharIt(sv))
        {
            print::out("({}): '{}' ({}), width: {}\n", i++, g, u32(g), wcWidth(g));
        }
    }

    print::out("\n");

    {
        /* even better */
        constexpr StringView sv = "Tëst 👨‍👩‍👦 🇺🇸 नी நி";
        print::out("({}) even better...\n", sv);
        int i = 0;
        print::out("wchars...\n");
        for (const wchar_t& g : StringWCharIt(sv))
        {
            print::out("({}): '{}' ({} / {:#x}), width: {}\n", i++, g, u32(g), u32(g), wcWidth(g));
        }
    }

    {
        String s0 = {&arena, "Release"};
        const String s1 = s0.release();
        print::out("s0: '{}', s1: '{}'\n", s0, s1);
        ADT_ASSERT_ALWAYS(s0.empty(), "");
        ADT_ASSERT_ALWAYS(s1 == "Release", "");
    }

    {
        struct DemangleVeryLong { struct IncreadiblyLongName { struct ActuallyWayTooLongNameItsCrazy {}; }; };
        DemangleVeryLong::IncreadiblyLongName::ActuallyWayTooLongNameItsCrazy demangle;
        print::out("demangled: '{}'\n", demangle);

        print::out("thread: '{}'\n", Thread {});
        print::out("StdAllocator: '{}'\n", StdAllocator {});
        print::out("arena: '{}'\n", arena);
    }

    {
        Vec<String> v0;
        rng::PCG32 rng = 666;
        for (isize i = 9; i >= 0; --i)
        {
            char aBuff[32] {};
            const isize n = print::toSpan(aBuff, "s{}", rng.nextInRange(0, 100));
            v0.emplace(&arena, &arena, aBuff, n);
        }

        sort::quick(&v0, utils::ComparatorRev<String> {});

        ADT_ASSERT_ALWAYS(StringView("s1") > StringView("s0"), "");
        ADT_ASSERT_ALWAYS(StringView("s0") < StringView("s1"), "");

        print::out("v0: {}\n", v0);
    }

    print::out("\n");

    {
        std::vector<std::string> v0;

        rng::PCG32 rng = 666;
        for (isize i = 9; i >= 0; --i)
        {
            char aBuff[32] {};
            const isize n = print::toSpan(aBuff, "s{}", rng.nextInRange(0, 100));
            v0.push_back({aBuff, size_t(n)});
        }

        sort::quick(&v0);

        print::out("std::vector: {}\n", v0);
    }

    print::out("StringTest: PASSED\n");
}
