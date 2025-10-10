#include "adt/Arena.hh"
#include "adt/Logger.hh"
#include "adt/Vec.hh"
#include "adt/ThreadPool.hh"
#include "adt/time.hh"

using namespace adt;

static void
lilBenchmark()
{
    constexpr isize BIG = 9999999;

    LogInfo{"lil bench (allocating in hot loop {} times)...\n", BIG};
    {
        time::Type timeIArenaIScope {};
        time::Type timeArenaIScope {};
        time::Type timeArena {};

        {
            auto& arena = *IThreadPool::inst()->arena();
            time::Type t0 = time::now();
            for (isize i = 0; i < BIG; ++i)
            {
                IArena::Scope arenaScope {&arena};
                char* pBuff = arena.mallocV<char>(300);
                print::toSpan({pBuff, 300}, "{}, {}, {}", i, std::pow(i, 2), BIG - i);
            }
            timeIArenaIScope = time::diffMSec(time::now(), t0);
            LogInfo{"virtual IArena + virtual IArena::IScope: {:.3} ms\n", timeIArenaIScope};
        }

        {
            Arena& arena = *dynamic_cast<Arena*>(IThreadPool::inst()->arena());
            time::Type t0 = time::now();
            for (isize i = 0; i < BIG; ++i)
            {
                auto arenaScope = arena.restoreAfterScope();
                char* pBuff = arena.mallocV<char>(300);
                print::toSpan({pBuff, 300}, "{}, {}, {}", i, std::pow(i, 2), BIG - i);
            }
            timeArenaIScope = time::diffMSec(time::now(), t0);
            LogInfo{"Non virtual Arena + virtual IArena::IScope: {:.3} ms\n", timeArenaIScope};
        }

        {
            Arena& arena = *dynamic_cast<Arena*>(IThreadPool::inst()->arena());
            time::Type t0 = time::now();
            for (isize i = 0; i < BIG; ++i)
            {
                IArena::Scope arenaScope {&arena};
                char* pBuff = arena.mallocV<char>(300);
                print::toSpan({pBuff, 300}, "{}, {}, {}", i, std::pow(i, 2), BIG - i);
            }
            timeArena = time::diffMSec(time::now(), t0);
            LogInfo{"Non virtual Arena + non virtual ArenaScope: {:.3} ms\n", timeArena};
        }

        LogInfo{"Non virtual Arena + non virtual ArenaScope is {:.2}% {}\n",
            (1.0 - ((f64)timeArena / (f64)timeIArenaIScope)) * 100.0,
            "faster/ slower"
        };
    }
    LogInfo{"lil bench finished\n"};
}

int
main()
{
    ThreadPool ztp {Arena{}, SIZE_1G*16};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Arena2 test...\n"};

    try
    {
        Arena& arena = static_cast<Arena&>(*IThreadPool::inst()->arena());
        lilBenchmark();
        arena.reset();

        {
            IArena::IScope topScope = arena.restoreAfterScope();

            LogInfo{"start off: {}\n", arena.memoryUsed()};

            Vec<isize> v0 {};
            Vec<isize> v1 {};

            for (isize i = 0; i < 100; ++i)
            {
                if (i & 1) v0.push(&arena, i);
                else v1.push(&arena, i);
            }

            for (auto e : v0) ADT_ASSERT_ALWAYS(e & 1, "e: {}", e);
            for (auto e : v1) ADT_ASSERT_ALWAYS(!(e & 1), "e: {}", e);

            LogInfo{"after push off: {}\n", arena.memoryUsed()};

            {
                LogInfo{"v0: {}\n", v0};
                LogInfo("off before pop: {}\n", arena.memoryUsed());
            }

            {
                LogInfo("v1: {}\n", v1);
                LogInfo("off before pop: {}\n", arena.memoryUsed());
            }
        }

        {
            IArena::Scope arenaScope {&arena};

            IArena::Ptr<i64> pNum {&arena, 999};

            LogInfo{"printing IArena::Ptr: '{}'\n", pNum};
        }

        {
            static int s_i = 0;
            static int s_magic = 0;

            struct Destructive
            {
                int m_i;
                const char* m_sv;

                Destructive(const char* nts) noexcept : m_sv{nts} { m_i = ++s_i; LogWarn{"({}) m_i: {}\n", nts, m_i}; }

                ~Destructive() noexcept
                {
                    LogDebug{"({}) {} dies...\n", m_sv, m_i};
                    --s_i;
                };

                void sayHi() noexcept { s_magic = 666; LogWarn{"{}({}) says hi\n", m_sv, m_i}; }
            };

            Arena::Ptr<Destructive> p;

            {
                IArena::IScope pushed = arena.restoreAfterScope();

                arena.initPtr(&p, "p");

                /* NOTE: triggers stack-use-after-scope with asan. */
                // Arena::Ptr<Destructive> p0 {Arena::Ptr<Destructive>::simpleDeleter, &arena, "p0"};
                // p0->sayHi();
            }
            LogDebug("offset after pop: {}\n", arena.memoryUsed());

            Arena::Ptr<Destructive> p3 {[](Arena::Ptr<Destructive>* pPtr) {
                pPtr->m_pData->~Destructive();
                pPtr->m_pData = nullptr;
            }, &arena, "p3"};

            p3->sayHi();

            /* BUG?: this assertion fails with DADT_LOGGER_LEVEL=-1 when compiling with clang -03,
             * (trying to elimitate dead code too aggressively?). */
            if (p)
            {
                LogError("p.m_pData: {}\n", p.m_pData);
                p->sayHi();
            }
            ADT_ASSERT_ALWAYS(!p, "!p: {}", !p);

            ADT_ASSERT_ALWAYS(s_i == 1, "i: {}", s_i);

            arena.reset();
            ADT_ASSERT_ALWAYS(!p3, "!p: {}", !p3);

            ADT_ASSERT_ALWAYS(s_magic == 666, "{}", s_magic);
        }
    }
    catch (const std::exception& ex)
    {
        LogDebug("{}\n", ex.what());
    }

    LogInfo{"Arena2 test passed\n"};
}
