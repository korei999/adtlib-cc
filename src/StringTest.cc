#include "adt/Arena.hh"
#include "adt/OsAllocator.hh"
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

    Arena arena(OsAllocatorGet(), SIZE_1K);
    defer( arena.freeAll() );

    String sHello = StringAlloc(&arena, "What is this");

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
        String s0 = "HELLO BIDEN";
        for (auto ch : ReverseIt(s0))
            putchar(ch);
        putchar('\n');
    }

    {
        const String s = "私は日本語を話せません";
        assert(s.endsWith("せません"));

        for (auto wc : StringGlyphIt(s))
            COUT("{}", wc);
        COUT("\n");
    }
}
