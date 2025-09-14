#include "adt/logs.hh"
#include "adt/ArenaList.hh"
#include "adt/defer.hh"
#include "adt/StdAllocator.hh"
#include "adt/Logger.hh"
#include "adt/time.hh"

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

    errno = {};
    auto* what = os.realloc((void*)0, 0, 0, 0);
    os.free(what);
    LOG_ERR("what: {}\n", what);
}

template<typename T>
struct ArenaSTD
{
    using value_type = T;

    ArenaList* m_pArena {};

    template<class U>
    constexpr ArenaSTD([[maybe_unused]] const ArenaSTD<U>& arena) noexcept {}

    ArenaSTD(ArenaList* pArena) noexcept
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

    ArenaList arena(SIZE_1K);
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
        f64 t0 = time::nowMS();
        for (int i = 0; i < big; ++i)
            vecStd.push_back(i);
        f64 t1 = time::nowMS() - t0;

        LOG("vecStd: {}\n", t1);
    }

    // {
    //    f64 t0 = time::nowMS();
    //     for (int i = 0; i < big; ++i)
    //         vecArena.push_back(i);
    //     f64 t1 = time::nowMS() - t0;

    //     LOG("vecArena: {}\n", t1);
    // }

    LOG("{}, {}\n", (int)std::numeric_limits<char>::min(), (int)std::numeric_limits<char>::max());
}
