#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"

#include <vector>

using namespace adt;

void
throws()
{
    OsAllocator osAl {};
    auto* ptr = osAl.zalloc(1, SIZE_8G * 1024);
    defer( osAl.free(ptr) );
}

template<typename T, typename ALLOC_T>
struct IAllocSTD
{
    using value_type = T;

    ALLOC_T* m_pArena {};

    explicit IAllocSTD(ALLOC_T* pArena) noexcept
        : m_pArena(pArena) {}

    T*
    allocate(std::size_t size)
    {
        return static_cast<T*>(m_pArena->malloc(size, sizeof(T)));
    }

    void
    deallocate(T*, std::size_t) noexcept
    {
        //
    }
};

int
main()
{
    try
    {
        throws();
    }
    catch (IException& pEx)
    {
        pEx.logErrorMsg();
    }

    Arena arena(SIZE_1K);
    /*FreeList arena(SIZE_1K);*/
    defer( arena.freeAll() );

    IAllocSTD<int, decltype(arena)> stdArena(&arena);

    std::vector<int, IAllocSTD<int, decltype(arena)>> vecArena(stdArena);
    std::vector<int> vecStd {};

    std::vector<int> vecStdWarmup {};

    {
        [[maybe_unused]] auto w0 = sizeof(vecArena);
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

    {
        f64 t0 = utils::timeNowMS();
        for (int i = 0; i < big; ++i)
            vecArena.push_back(i);
        f64 t1 = utils::timeNowMS() - t0;

        LOG("vecArena: {}\n", t1);
    }

    LOG("{}, {}\n", (int)std::numeric_limits<char>::min(), (int)std::numeric_limits<char>::max());
}
