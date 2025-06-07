#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ReverseIt.hh"

#include <clocale>

using namespace adt;

int
main()
{
    setlocale(LC_ALL, "");

    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    String sHello = String(&arena, "What is this");

    bool bNull = sHello.contains("");

    bool bWhat = sHello.contains("What");
    bool bIs = sHello.contains("is");
    bool bThis = sHello.contains("this");

    bool bWh = sHello.contains("Wh");
    bool bHis = sHello.contains("his");

    bool bHWha = sHello.contains("HWha");

    LOG("null: {}, What: {}, is: {}, this: {}, Wh: {}, his: {}, HWha: {}\n", bNull, bWhat, bIs, bThis, bWh, bHis, bHWha);

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
        COUT("s: '{}'\n", s);

        COUT("\"");
        for (auto wc : StringWCharIt(s))
            COUT("'{}'", wc);
        COUT("\"\n");
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
        constexpr StringView s = "🇺🇦КВИТКИ";
        int i = 0;
        COUT("glyphs...\n");
        for (const wchar_t& g : StringWCharIt(s))
        {
            COUT("({}): '{}' ({}), width: {}\n", i++, g, u32(g), wcWidth(g));
        }

        i = 0;
        COUT("graphemes...\n");
        for (const StringView sv : StringGraphemeIt(s))
        {
            COUT("grapheme #{}: '{}' (size: {})\n", i++, sv, sv.size());
        }
    }

    {
        String s0 = {&arena, "Release"};
        const String s1 = s0.release();
        COUT("s0: '{}', s1: '{}'\n", s0, s1);
        ADT_ASSERT_ALWAYS(s0.empty(), "");
        ADT_ASSERT_ALWAYS(s1 == "Release", "");
    }

    COUT("StringTest: PASSED\n");
}
