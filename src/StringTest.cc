#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/String.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
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
}
