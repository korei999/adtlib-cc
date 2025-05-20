#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/String.hh"
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
        for (auto wc : StringGlyphIt(s))
            COUT("'{}'", wc);
        COUT("\"\n");
    }

    {
        constexpr StringView s = "word0 word1 word2 word3";
        for (const auto& sWord : StringWordIt(s))
        {
            COUT("'{}'\n", sWord);
        }
    }

    {
        constexpr StringView s = "comma0,comma1,comma2,comma3";
        for (const auto& sWord : StringWordIt(s, ","))
        {
            COUT("'{}'\n", sWord);
        }
    }

    {
        constexpr StringView s = "STR0 | STR1 | STR2";
        for (const auto& svWord : StringWordIt(s, " |"))
        {
            COUT("'{}'\n", svWord);
        }
    }

    {
        constexpr StringView s = "🇺🇦КВИТКИ";
        int i = 0;
        COUT("glyphs...\n");
        for (const wchar_t& g : StringGlyphIt(s))
        {
            COUT("({}): '{}' ({}), width: {}\n", i++, g, u32(g), wcwidth(g));
        }

        i = 0;
        COUT("graphemes...\n");
        for (const StringView sv : StringGraphemeIt(s))
        {
            COUT("grapheme #{}: '{}'\n", i++, sv);
        }
    }
}
