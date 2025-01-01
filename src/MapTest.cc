#include "adt/OsAllocator.hh" /* IWYU pragma: keep */
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

    Map<String, u32> map(&arena);

    map.insert("ThirdyTwo", 32);
    map.insert("Sixteen", 16);
    map.insert("Seventeen", 17);
    map.insert("FiftyFive", 55);
    map.emplace("NineHundredNinetyNine", 999);

    map.remove("Seventeen");
    map.remove("FiftyFive");

    {
        auto fSixteen = map.search("Sixteen");
        assert(fSixteen);
        if (fSixteen) LOG("found: {}\n", fSixteen.data());
    }
    {
        auto fThirdyTwo = map.search("ThirdyTwo");
        assert(fThirdyTwo);
        if (fThirdyTwo) LOG("found: {}\n", fThirdyTwo.data());
    }
    {
        auto fSeventeen = map.search("Seventeen");
        assert(fSeventeen == false);
        if (fSeventeen) LOG("found: {}\n", fSeventeen.data());
    }
    {
        auto fFiftyFive = map.search("FiftyFive");
        assert(fFiftyFive == false);
        if (fFiftyFive) LOG("found: {}\n", fFiftyFive.data());
    }
    {
        auto fNineHundredNinetyNine = map.search("NineHundredNinetyNine");
        assert(fNineHundredNinetyNine);
        if (fNineHundredNinetyNine) LOG("found: {}\n", fNineHundredNinetyNine.data());
    }

    COUT("map auto loop: ");
    for (auto& [k, v] : map)
        COUT("['{}', {}], ", k, v);
    COUT("\n");

    Map<int, int> map2(&arena);
    map2.insert(12, 1);
    map2.insert(13, 2);

    {
        auto one = map2.search(12);
        assert(one);
        LOG("one: {}\n", one.data());
    }

    {
        auto two = map2.search(13);
        assert(two);
        LOG("one: {}\n", two.data());
    }
}
