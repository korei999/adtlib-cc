#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/Arena.hh"
#include "adt/Map.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );
    arena.freeAll();

    Map<String, u32> map(&arena.super);

    map.insert("ThirdyTwo", 32);
    map.insert("Sixteen", 16);
    map.insert("Seventeen", 17);

    map.remove("Seventeen");

    {
        auto fSixteen = map.search("Sixteen");
        if (fSixteen) LOG("found: {}, {}\n", fSixteen.pData->key, fSixteen.pData->val);
    }
    {
        auto fThirdyTwo = map.search("ThirdyTwo");
        if (fThirdyTwo) LOG("found: {}, {}\n", fThirdyTwo.pData->key, fThirdyTwo.pData->val);
    }
    {
        auto fSeventeen = map.search("Seventeen");
        if (fSeventeen) LOG("found: {}, {}\n", fSeventeen.pData->key, fSeventeen.pData->val);
        else LOG("Seventeen: not found\n");
    }
}
