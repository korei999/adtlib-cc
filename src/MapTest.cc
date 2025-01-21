#include "adt/OsAllocator.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/Span.hh"

using namespace adt;

enum CONSTEXPR_HASH : usize
{
    CONST_ = hash::func("CONST"),
    EXPR_ = hash::func("EXPR"),
};

static usize
memeHash(const int& x)
{
    return usize(x);
}

static String
genRandomString(IAllocator* pAlloc)
{
    const char* ntsChars = "1234567890-=qwertyuiop[]asdfghjklQWERTASDVZXCVKLJ:H";
    ssize len = strlen(ntsChars);

    ssize size = (rand() % (len-2)) + 2;
    auto s = StringAlloc(pAlloc, size);

    for (auto& ch : s)
        ch = ntsChars[ rand() % len ];

    return s;
}

static void
microBench()
{
    Arena arena(SIZE_8M * 10);
    defer( arena.freeAll() );

    constexpr ssize BIG = 1000000;

    Vec<String> vStrings(&arena, BIG);
    vStrings.setSize(BIG);

    for (ssize i = 0; i < BIG; ++i)
        vStrings[i] = genRandomString(&arena);

    Map<String, int> map(&arena);

    {
        f64 t0 = utils::timeNowMS();

        for (ssize i = 0; i < BIG; ++i)
            map.tryInsert(vStrings[i], i);

        f64 t1 = utils::timeNowMS() - t0;
        LOG("tryInsert {} items in {} ms\n", BIG, t1);
    }

    {
        f64 t0 = utils::timeNowMS();

        for (ssize i = 0; i < BIG; ++i)
        {
            [[maybe_unused]] auto f = map.search(vStrings[i]);
        }

        f64 t1 = utils::timeNowMS() - t0;
        LOG("search {} items in {} ms\n", BIG, t1);
    }
}

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

    Map<int, int, memeHash> map2(&arena);
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
        LOG("two: {}\n", two.data());
    }

    int buff[123] {};
    Span sp(buff);
    hash::func(sp);

    hash::func(String("asdf"));
    String asdf = "asdf";
    hash::func(asdf);
    hash::func("asdf");

    microBench();
}
