#include "adt/StdAllocator.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/Span.hh"
#include "adt/rng.hh"

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

static rng::PCG32 s_rng {usize(utils::timeNowUS())};

static String
genRandomString(IAllocator* pAlloc)
{
    const char* ntsChars = "1234567890-=qwertyuiop[]asdfghjklQWERTASDVZXCVKLJ:H";
    isize len = strlen(ntsChars);

    isize size = (s_rng.next() % (len-2)) + 2;
    auto pMem = pAlloc->zallocV<char>(size);
    auto s = String(pAlloc, pMem, size);

    for (auto& ch : s)
        ch = ntsChars[ s_rng.next() % len ];

    return s;
}

static void
microBench()
{
    Arena arena(SIZE_8M * 10);
    defer( arena.freeAll() );

    constexpr isize BIG = 1000000;

    VecPmr<String> vStrings {&arena, BIG};
    vStrings.setSize(BIG);

    for (isize i = 0; i < BIG; ++i)
        vStrings[i] = genRandomString(&arena);

    MapPmr<StringView, int> map(&arena);
    VecPmr<StringView> vNotFoundStrings {StdAllocator::inst()};
    defer( vNotFoundStrings.destroy() );

    {
        f64 t0 = utils::timeNowMS();

        for (isize i = 0; i < BIG; ++i)
        {
            auto res = map.tryInsert(vStrings[i], i);
            if (res.eStatus == MAP_RESULT_STATUS::FOUND)
            {
                vNotFoundStrings.push(vStrings[i]);
            }
        }

        f64 t1 = utils::timeNowMS() - t0;
        LOG("tryInsert {} items in {} ms\n", BIG, t1);
    }

    {
        f64 t0 = utils::timeNowMS();

        for (isize i = 0; i < BIG; ++i)
        {
            [[maybe_unused]] auto f = map.search(vStrings[i]);
            if (!f)
            {
                auto onceMore = map.search(vStrings[i]);

                bool bFound = false;
                for (auto& bucket : map.m_vBuckets)
                {
                    if (bucket.key == vStrings[i])
                    {
                        bFound = true;
                        break;
                    }
                }
                LOG_WARN("bFound: {}, '{}': str: '{}', onceMore: {}\n", bFound, f.eStatus, vStrings[i], onceMore.eStatus);
            }
        }

        f64 t1 = utils::timeNowMS() - t0;
        LOG("search {} items in {} ms\n", BIG, t1);

        for (auto& sv : vNotFoundStrings)
        {
            ADT_ASSERT_ALWAYS(map.search(sv), "failed to find: '{}'", sv);
        }
    }
}

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    ADT_ASSERT_ALWAYS(hash::func("]e") == hash::func(StringView("]e")), "");

    {
        MapPmr<StringView, int> map {&arena};
        map.insert("OneTwoThree", 123);
        map.insert("OneTwoThree", 1234);

        for (auto& kv : map)
        {
            CERR("kv: {}\n", kv);
        }
    }

    MapPmr<StringView, u32> map(&arena);

    map.insert("ThirdyTwo", 32);
    map.insert("Sixteen", 16);
    map.insert("Seventeen", 17);
    map.insert("FiftyFive", 55);
    map.emplace("NineHundredNinetyNine", 999);

    map.remove("Seventeen");
    map.remove("FiftyFive");

    {
        auto fSixteen = map.search("Sixteen");
        ADT_ASSERT_ALWAYS(fSixteen, "");
        if (fSixteen) LOG("found: {}\n", fSixteen.data());
    }
    {
        auto fThirdyTwo = map.search("ThirdyTwo");
        ADT_ASSERT_ALWAYS(fThirdyTwo, "");
        if (fThirdyTwo) LOG("found: {}\n", fThirdyTwo.data());
    }
    {
        auto fSeventeen = map.search("Seventeen");
        ADT_ASSERT_ALWAYS(fSeventeen.eStatus == MAP_RESULT_STATUS::NOT_FOUND, "");
        if (fSeventeen) LOG("found: {}\n", fSeventeen.data());
    }
    {
        auto fFiftyFive = map.search("FiftyFive");
        ADT_ASSERT_ALWAYS(fFiftyFive.eStatus == MAP_RESULT_STATUS::NOT_FOUND, "");
        if (fFiftyFive) LOG("found: {}\n", fFiftyFive.data());
    }
    {
        auto fNineHundredNinetyNine = map.search("NineHundredNinetyNine");
        ADT_ASSERT_ALWAYS(fNineHundredNinetyNine, "");
        if (fNineHundredNinetyNine) LOG("found: {}\n", fNineHundredNinetyNine.data());
    }

    COUT("map auto loop: ");
    for (auto& [k, v] : map)
        COUT("['{}', {}], ", k, v);
    COUT("\n");

    MapPmr<int, int, memeHash> map2(&arena);
    map2.insert(12, 1);
    map2.insert(13, 2);

    {
        auto one = map2.search(12);
        ADT_ASSERT_ALWAYS(one, "");
        LOG("one: {}\n", one.data());
    }

    {
        auto two = map2.search(13);
        ADT_ASSERT_ALWAYS(two, "");
        LOG("two: {}\n", two.data());
    }

    int buff[123] {};
    Span sp(buff);
    hash::func(sp);

    hash::func(StringView("asdf"));
    StringView asdf = "asdf";
    hash::func(asdf);
    hash::func("asdf");

    microBench();
}
