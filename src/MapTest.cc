#include "adt/StdAllocator.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/Span.hh" /* IWYU pragma: keep */
#include "adt/rng.hh"
#include "adt/time.hh"

#include <string_view> /* IWYU pragma: keep */
#include <string>
#include <unordered_map>

using namespace adt;

static const usize CONSTEXPR_HASH_CONST = hash::func("CONST");
static const usize CONSTEXPR_HASH_EXPR = hash::func("EXPR");

static usize
memeHash(const int& x)
{
    return usize(x);
}

static rng::PCG32 s_rng {usize(time::nowUS())};

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

    Vec<String> vStrings {&arena, BIG};
    vStrings.setSize(&arena, BIG);

    for (isize i = 0; i < BIG; ++i)
        vStrings[i] = genRandomString(&arena);

    {
        MapM<int, int> map {};
        defer( map.destroy() );
        map.insert(1, 2);
        map.insert(2, 3);
        map.insert(3, 4);

        LOG("map<int, int>: {}\n", map);
    }

    {
        Map<StringView, int> map(&arena);
        VecManaged<StringView> vNotFoundStrings {};
        defer( vNotFoundStrings.destroy() );

        {
            f64 t0 = time::nowMS();

            for (isize i = 0; i < BIG; ++i)
            {
                auto res = map.tryInsert(&arena, vStrings[i], i);
                if (res.eStatus == MAP_RESULT_STATUS::FOUND)
                {
                    vNotFoundStrings.push(vStrings[i]);
                }
            }

            f64 t1 = time::nowMS() - t0;
            LOG("tryInsert {} items in {} ms\n", BIG, t1);
        }

        {
            f64 t0 = time::nowMS();

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

            f64 t1 = time::nowMS() - t0;
            LOG("search {} items in {} ms\n", BIG, t1);

            for (auto& sv : vNotFoundStrings)
            {
                ADT_ASSERT_ALWAYS(map.search(sv), "failed to find: '{}'", sv);
            }
        }
    }

    CERR("\n");

    {
        struct StringViewHash
        {
            std::size_t operator()(const StringView sv) const
            {
                return hash::func(sv);
                // return std::hash<std::string_view> {}(std::string_view {sv.data(), std::size_t(sv.size())});
            };
        };

        std::unordered_map<StringView, int, StringViewHash> map;
        VecM<StringView> vNotFoundStrings {};
        defer( vNotFoundStrings.destroy() );

        {
            f64 t0 = time::nowMS();

            for (isize i = 0; i < BIG; ++i)
            {
                auto res = map.try_emplace(vStrings[i], i);
                if (!res.second)
                    vNotFoundStrings.push(vStrings[i]);
            }

            f64 t1 = time::nowMS() - t0;
            LOG("STL: try_emplace {} items in {} ms\n", BIG, t1);
        }

        {
            f64 t0 = time::nowMS();

            for (isize i = 0; i < BIG; ++i)
            {
                [[maybe_unused]] auto f = map.find(vStrings[i]);
                if (f == map.end())
                {
                    auto onceMore = map.find(vStrings[i]);

                    bool bFound = false;
                    for (auto& bucket : map)
                    {
                        if (bucket.first == vStrings[i])
                        {
                            bFound = true;
                            break;
                        }
                    }
                    LOG_WARN("bFound: {}, '{}': str: '{}', onceMore: {}\n", bFound, f->second, vStrings[i], onceMore->second);
                }
            }

            f64 t1 = time::nowMS() - t0;
            LOG("STL: search {} items in {} ms\n", BIG, t1);

            for (auto& sv : vNotFoundStrings)
            {
                ADT_ASSERT_ALWAYS(map.find(sv) != map.end(), "failed to find: '{}'", sv);
            }
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
        Map<StringView, int> map {&arena};
        map.insert(&arena, "OneTwoThree", 123);
        map.insert(&arena, "OneTwoThree", 1234);

        for (auto& kv : map)
        {
            CERR("kv: {}\n", kv);
        }
    }

    {
        Map<std::string, int> map {&arena};
        defer( map.destroy(&arena) );

        map.insert(&arena, "one", 1);
        map.insert(&arena, "two", 2);
        map.insert(&arena, "three", 3);
        map.insert(&arena, "four", 4);
        map.insert(&arena, "five", 5);
        map.insert(&arena, "six", 6);
    }

    Map<StringView, u32> map(&arena);

    map.insert(&arena, "ThirdyTwo", 32);
    map.insert(&arena, "Sixteen", 16);
    map.insert(&arena, "Seventeen", 17);
    map.insert(&arena, "FiftyFive", 55);
    map.emplace(&arena, "NineHundredNinetyNine", 999);

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

    Map<int, int, memeHash> map2(&arena);
    map2.insert(&arena, 12, 1);
    map2.insert(&arena, 13, 2);

    {
        auto one = map2.search(12);
        ADT_ASSERT_ALWAYS(one, "");
        LOG("one: {}\n", one);
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
