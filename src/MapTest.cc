#include "adt/Gpa.hh" /* IWYU pragma: keep */
#include "adt/defer.hh"
#include "adt/Map.hh"
#include "adt/Span.hh" /* IWYU pragma: keep */
#include "adt/rng.hh"
#include "adt/time.hh"
#include "adt/Arena.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

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

static rng::PCG32 s_rng {(u64)time::now()};

[[maybe_unused]] static usize
somethingHash(const StringView& sv)
{
    isize i = 0;
    usize hash = 0;
    constexpr u64 randomGiantNumber = 0xf9135213895caf14LLU;
    for (; i + 7 < sv.m_size; i += 8)
    {
        u64 x = sv.reinterpret<u64>(i);
        hash += (x * randomGiantNumber) ^ x;
    }

    if (sv.m_size >= 8)
    {
        u64 x = sv.reinterpret<u64>(sv.m_size - 8);
        hash += (x * randomGiantNumber) ^ x;
    }
    else
    {
        for (; i + 3 < sv.m_size; i += 4)
        {
            u64 x = sv.reinterpret<u32>(i);
            hash += (x * randomGiantNumber) ^ x;
        }
        for (; i + 1 < sv.m_size; i += 2)
        {
            u64 x = sv.reinterpret<u16>(i);
            hash += (x * randomGiantNumber) ^ x;
        }
        for (; i < sv.m_size; ++i)
        {
            hash += (sv[i] * randomGiantNumber) ^ sv[i];
        }
    }

    return hash;
}

static String
genRandomString(IAllocator* pAlloc)
{
    const char* ntsChars = "1234567890-=qwertyuiop[]asdfghjklQWERTASDVZXCVKLJ:H";
    isize len = strlen(ntsChars);

    isize size = (s_rng.next() % (len-2)) + 2;
    auto pMem = pAlloc->zallocV<char>(size);
    auto s = String(pAlloc, pMem, size);

    for (auto& ch : s) ch = ntsChars[ s_rng.next() % len ];

    return s;
}

static void
microBench()
{
    IArena& arena = *IThreadPool::inst()->arena();

    constexpr isize BIG = 1000000;

    Vec<String> vStrings {&arena, BIG};
    vStrings.setSize(&arena, BIG);

    for (isize i = 0; i < BIG; ++i) vStrings[i] = genRandomString(&arena);

    {
        MapM<int, int> map {};
        defer( map.destroy() );
        map.insert(1, 2);
        map.insert(2, 3);
        map.insert(3, 4);

        LogDebug("map<int, int>: {}\n", map);
    }

    {
        Map<StringView, int> map(&arena);
        VecManaged<StringView> vNotFoundStrings {};
        defer( vNotFoundStrings.destroy() );

        {
            auto timer = time::now();

            for (isize i = 0; i < BIG; ++i)
            {
                auto res = map.tryInsert(&arena, vStrings[i], i);
                if (res.eStatus == MAP_RESULT_STATUS::FOUND)
                {
                    vNotFoundStrings.push(vStrings[i]);
                }
            }

            LogDebug("tryInsert {} items in {:.3} ms\n", BIG, time::diffMSec(time::now(), timer));
        }

        {
            auto timer = time::now();

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
                    LogWarn("bFound: {}, '{}': str: '{}', onceMore: {}\n", bFound, f.eStatus, vStrings[i], onceMore.eStatus);
                }
            }

            LogDebug("search {} items in {:.3} ms\n", BIG, time::diffMSec(time::now(), timer));

            for (auto& sv : vNotFoundStrings)
            {
                ADT_ASSERT_ALWAYS(map.search(sv), "failed to find: '{}'", sv);
            }
        }
    }

    LogDebug("\n");

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
            auto timer = time::now();

            for (isize i = 0; i < BIG; ++i)
            {
                auto res = map.try_emplace(vStrings[i], i);
                if (!res.second)
                    vNotFoundStrings.push(vStrings[i]);
            }

            LogDebug("STL: try_emplace {} items in {:.3} ms\n", BIG, time::diffMSec(time::now(), timer));
        }

        {
            auto timer = time::now();

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
                    LogWarn("bFound: {}, '{}': str: '{}', onceMore: {}\n", bFound, f->second, vStrings[i], onceMore->second);
                }
            }

            LogDebug("STL: search {} items in {:.3} ms\n", BIG, time::diffMSec(time::now(), timer));

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
    ThreadPool ztp {Arena{}, SIZE_1G};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    IArena& arena = *ztp.arena();

    ADT_ASSERT_ALWAYS(hash::func("]e") == hash::func(StringView("]e")), "");

    {
        Map<StringView, int> map {&arena};
        map.insert(&arena, "OneTwoThree", 123);
        map.insert(&arena, "OneTwoThree", 1234);

        for (auto& kv : map)
        {
            LogDebug("kv: {}\n", kv);
        }
    }

    {
        IArena::Scope arenaScope {&arena};
        Map<std::string, int> map {&arena};
        defer( map.destroy(&arena) );

        map.insert(&arena, "one", 1);
        map.insert(&arena, "two", 2);
        map.insert(&arena, "three", 3);
        map.insert(&arena, "four", 4);
        map.insert(&arena, "five", 5);
        map.insert(&arena, "six", 6);
    }

    {
        IArena::IScope astate = arena.restoreAfterScope();
        Map<StringView, u32> mapWithInitializerList {&arena,{
            {"one", 1},
            {"two", 2},
            {"three", 3},
            {"four", 4},
            {"five", 5},
            {"six", 6},
            {"seven", 7},
        }};

        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("one").value() == 1, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("two").value() == 2, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("three").value() == 3, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("four").value() == 4, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("five").value() == 5, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("six").value() == 6, "");
        ADT_ASSERT_ALWAYS(mapWithInitializerList.search("seven").value() == 7, "");

        print::out("mapWithInitializerList: {}\n", mapWithInitializerList);
    }

    {
        IArena::IScope arenaScope = arena.restoreAfterScope();
        Map<char const*, int, hash::nullTermStringFunc> mapNtsToInt {&arena, {
            {"one", 1},
            {"two", 2},
            {"three", 3},
            {"four", 4},
            {"five", 5},
            {"six", 6},
            {"seven", 7},
        }};

        ADT_ASSERT_ALWAYS(mapNtsToInt.search("one").value() == 1, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("two").value() == 2, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("three").value() == 3, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("four").value() == 4, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("five").value() == 5, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("six").value() == 6, "");
        ADT_ASSERT_ALWAYS(mapNtsToInt.search("seven").value() == 7, "");

        print::out("mapNtsToInt: {}\n", mapNtsToInt);
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
        if (fSixteen) LogDebug("found: {}\n", fSixteen.data());
    }
    {
        auto fThirdyTwo = map.search("ThirdyTwo");
        ADT_ASSERT_ALWAYS(fThirdyTwo, "");
        if (fThirdyTwo) LogDebug("found: {}\n", fThirdyTwo.data());
    }
    {
        auto fSeventeen = map.search("Seventeen");
        ADT_ASSERT_ALWAYS(fSeventeen.eStatus == MAP_RESULT_STATUS::NOT_FOUND, "");
        if (fSeventeen) LogDebug("found: {}\n", fSeventeen.data());
    }
    {
        auto fFiftyFive = map.search("FiftyFive");
        ADT_ASSERT_ALWAYS(fFiftyFive.eStatus == MAP_RESULT_STATUS::NOT_FOUND, "");
        if (fFiftyFive) LogDebug("found: {}\n", fFiftyFive.data());
    }
    {
        auto fNineHundredNinetyNine = map.search("NineHundredNinetyNine");
        ADT_ASSERT_ALWAYS(fNineHundredNinetyNine, "");
        if (fNineHundredNinetyNine) LogDebug("found: {}\n", fNineHundredNinetyNine.data());
    }

    {
        IArena::IScope arenaScope = arena.restoreAfterScope();

        print::Builder pb {&arena};
        for (auto& [k, v] : map) pb.print("('{}', {}), ", k, v);
        LogDebug("map auto loop: {}\n", StringView(pb));
    }

    Map<int, int, memeHash> map2(&arena);
    map2.insert(&arena, 12, 1);
    map2.insert(&arena, 13, 2);

    {
        auto one = map2.search(12);
        ADT_ASSERT_ALWAYS(one, "");
        LogDebug("one: {}\n", one);
    }

    {
        auto two = map2.search(13);
        ADT_ASSERT_ALWAYS(two, "");
        LogDebug("two: {}\n", two.data());
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
