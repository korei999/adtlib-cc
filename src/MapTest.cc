#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Map.hh"

using namespace adt;

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    Map<String, u32> map(&arena.super);

    map.insert("ThirdyTwo", 32);
    map.insert("Sixteen", 16);
    map.insert("Seventeen", 17);
    map.insert("FiftyFive", 55);
    map.insert("NineHundredNinetyNine", 999);

    map.remove("Seventeen");
    map.remove("FiftyFive");

    {
        auto fSixteen = map.search("Sixteen");
        assert(fSixteen);
        if (fSixteen) LOG("found: {}\n", fSixteen.getData());
    }
    {
        auto fThirdyTwo = map.search("ThirdyTwo");
        assert(fThirdyTwo);
        if (fThirdyTwo) LOG("found: {}\n", fThirdyTwo.getData());
    }
    {
        auto fSeventeen = map.search("Seventeen");
        assert(fSeventeen == false);
        if (fSeventeen) LOG("found: {}\n", fSeventeen.getData());
    }
    {
        auto fFiftyFive = map.search("FiftyFive");
        assert(fFiftyFive == false);
        if (fFiftyFive) LOG("found: {}\n", fFiftyFive.getData());
    }
    {
        auto fNineHundredNinetyNine = map.search("NineHundredNinetyNine");
        assert(fNineHundredNinetyNine);
        if (fNineHundredNinetyNine) LOG("found: {}\n", fNineHundredNinetyNine.getData());
    }
}
