import adt.Malloc;
import adt.print;
import adt.Vec;
import adt.Arena;
import adt.Map;
import adt.String;
import adt.PoolSOA;
import adt.ScratchBuffer;

#include "adt/defer.hh"
#include "adt/types.hh"

using namespace adt;

thread_local static u8 tls_aScratchBuff[SIZE_1M] {};
thread_local static ScratchBuffer tls_scratch(tls_aScratchBuff);

struct Test
{
    int x;
    int y;
};

struct TestBind
{
    int& x;
    int& y;
};

int
main()
{
    Span<Arena> spArenas = tls_scratch.nextMem<Arena>(1);
    new(&spArenas[0]) Arena(SIZE_1K);
    defer ( spArenas[0].freeAll() );

    VecManaged<int> vec(&spArenas[0]);
    vec.push(1);
    vec.push(-1);
    vec.push(2);
    vec.push(-2);

    for (int e : vec)
        print::out("e: {}\n", e);
    print::out("\n");

    for (int e : Span<int>(vec))
        print::out("e: {}\n", e);

    MapManaged<StringView, int> map(&spArenas[0], 100);
    map.insert("one", 1);
    map.insert("two", 2);
    map.insert("three", 3);
    map.insert("four", 4);

    print::out("one: {}:\n", map.search("one").valueOr(-1));
    print::out("two: {}:\n", map.search("two").valueOr(-1));
    print::out("tree: {}:\n", map.search("three").valueOr(-1));
    print::out("four: {}:\n", map.search("four").valueOr(-1));

    PoolSOA<Test, TestBind, 100, &Test::x, &Test::y> poolSOA;
    PoolSOAHandle<Test> poolHnd = poolSOA.make({});
}
