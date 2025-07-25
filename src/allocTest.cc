#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"
#include "adt/StdAllocator.hh"

#include <vector>
#include <limits>

using namespace adt;

void
throws()
{
    StdAllocator osAl {};
    auto* ptr = osAl.zalloc(1, SIZE_8G * 1024);
    defer( osAl.free(ptr) );
}

static void
reallocZero()
{
    StdAllocator os {};
    FreeList fl(500);;

    errno = {};
    auto* what = os.realloc((void*)0, 0, 0, 0);
    os.free(what);
    LOG_ERR("what: {}\n", what);

    errno = {};
    auto* whatFl = fl.realloc(nullptr, 0, 0, 0);
    fl.free(whatFl);
    LOG_ERR("whatFl: {}\n", whatFl);
}

template<typename T>
struct ArenaSTD
{
    using value_type = T;

    Arena* m_pArena {};

    template<class U>
    constexpr ArenaSTD(const ArenaSTD<U>& arena) noexcept {}

    ArenaSTD(Arena* pArena) noexcept
        : m_pArena(pArena) {}

    T*
    allocate(std::size_t size)
    {
        return static_cast<T*>(m_pArena->malloc(size, sizeof(T)));
    }

    void
    dealloc(T*, std::size_t) noexcept
    {
        //
    }
};

template<class T, class U>
bool operator==(const ArenaSTD<T>&, const ArenaSTD<U>&) { return true; }

template<class T, class U>
bool operator!=(const ArenaSTD<T>&, const ArenaSTD<U>&) { return false; }

int
main()
{
    try
    {
        reallocZero();
        throws();
    }
    catch (IException& pEx)
    {
        pEx.printErrorMsg(stderr);
    }

    Arena arena(SIZE_1K);
    /*FreeList arena(SIZE_1K);*/
    defer( arena.freeAll() );

    ArenaSTD<int> stdArena(&arena);

    /*std::vector<int, ArenaSTD<int>> vecArena(stdArena);*/
    std::vector<int> vecStd {};

    std::vector<int> vecStdWarmup {};

    {
        /*[[maybe_unused]] auto w0 = sizeof(vecArena);*/
        [[maybe_unused]] auto w1 = sizeof(vecStd);
    }

    int big = 10000000;

    {
        for (int i = 0; i < big; ++i)
            vecStdWarmup.push_back(i);
    }

    {
        f64 t0 = utils::timeNowMS();
        for (int i = 0; i < big; ++i)
            vecStd.push_back(i);
        f64 t1 = utils::timeNowMS() - t0;

        LOG("vecStd: {}\n", t1);
    }

    // {
    //    f64 t0 = utils::timeNowMS();
    //     for (int i = 0; i < big; ++i)
    //         vecArena.push_back(i);
    //     f64 t1 = utils::timeNowMS() - t0;

    //     LOG("vecArena: {}\n", t1);
    // }

    LOG("{}, {}\n", (int)std::numeric_limits<char>::min(), (int)std::numeric_limits<char>::max());
}
