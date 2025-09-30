#include "adt/ArenaList.hh"
#include "adt/defer.hh"
#include "adt/Gpa.hh"
#include "adt/Logger.hh"
#include "adt/time.hh"
#include "adt/ThreadPool.hh"

#include <vector>
#include <limits>

using namespace adt;

void
throws()
{
    Gpa osAl {};
    auto* ptr = osAl.zalloc(1, SIZE_8G * 1024);
    defer( osAl.free(ptr) );
}

static void
reallocZero()
{
    Gpa os {};

    errno = {};
    auto* what = os.realloc((void*)0, 0, 0, 0);
    os.free(what);
    LogError("what: {}\n", what);
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
    ThreadPool ztp {SIZE_1G};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    try
    {
        reallocZero();
        throws();
    }
    catch (const std::exception& ex)
    {
        print::err("{}\n", ex.what());
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
        auto timer = time::now();
        for (int i = 0; i < big; ++i)
            vecStd.push_back(i);
        LogDebug("vecStd: {}\n", time::diffMSec(time::now(), timer));
    }

    // {
    //    f64 t0 = time::nowMS();
    //     for (int i = 0; i < big; ++i)
    //         vecArena.push_back(i);
    //     f64 t1 = time::nowMS() - t0;

    //     LogDebug("vecArena: {}\n", t1);
    // }

    LogDebug("{}, {}\n", (int)std::numeric_limits<char>::min(), (int)std::numeric_limits<char>::max());
}
