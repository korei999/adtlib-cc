#include "adt/RefCount.hh"
#include "adt/ArenaList.hh"
#include "adt/Logger.hh"

#include "Types.hh"

#include "WIP/math.hh"

using namespace adt;

template<typename T>
static void
show(RefCountedPtr<T> rcp)
{
    if (!rcp)
    {
        LogWarn("!rcp\n");
        return;
    }
    RefScope refScope {&rcp};

    LogInfo("rcp: {}\n", *rcp);
}

int
main()
{
    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("RefCount test...\n");

    {
        RefCountedPtr<int> rcp = RefCountedPtr<int>::alloc(1);
        defer( rcp.unref() );

        RefCountedPtr<int> rcp2 = rcp.ref();
        defer( rcp.unref() );

        show(rcp);

        *rcp = 2;
        ADT_ASSERT_ALWAYS(*rcp == *rcp2, "{}, {}", *rcp, *rcp2);

        show(rcp2);
    }

    {
        WeakPtr<What> wp;
        defer( wp.weakUnref() );

        {
            auto rcp = RefCountedPtr<What>::alloc(1);
            defer( rcp.unref() );

            RefCountedPtr<What> rcp2 = rcp.ref();
            defer( rcp.unref() );

            wp = rcp2.weakRef();

            show(rcp2);

            show(wp);
        }

        show(wp);
    }

    {
        RefCountedPtr<math::V3> rcp = Gpa::inst()->alloc<math::V3>(math::V3{1.1f, 2.2f, 3.3f});
        defer( rcp.unref() );

        auto rcp2 = rcp.ref();
        defer( rcp2.unref() );

        show(rcp);

        *rcp = math::V3{6.6f, 7.7f, 8.8f};
        ADT_ASSERT_ALWAYS(*rcp2 == *rcp, "{}, {}", *rcp2, *rcp);

        show(rcp2);
    }

    {
        auto clDeleter = [](ArenaList* p) noexcept {
            p->freeAll();
        };

        auto rcpArena = RefCountedPtr<ArenaList>::allocWithDeleter(clDeleter, ArenaList{SIZE_1K});
        defer( rcpArena.unref() );
    }

    {
        auto clDeleter = [](ArenaList* p) noexcept {
            p->freeAll();
            ::free(p);
        };

        RefCountedPtr<ArenaList> rpcArena {clDeleter, Gpa::inst()->alloc<ArenaList>(SIZE_1K)};
        defer( rpcArena.unref() );
    }

    LogInfo("RefCount test passed\n");
}
