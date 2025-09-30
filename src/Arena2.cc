#include "adt/Arena.hh"
#include "adt/Logger.hh"
#include "adt/Vec.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Arena2 test...\n"};

    try
    {
        Arena& arena = *IThreadPool::inst()->arena();

        {
            ArenaScope pushedTop {&arena};

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
                ArenaScope pushed {&arena};

                arena.initPtr(&p, "p");

                /* BUG?: triggers stack-use-after-scope with asan. */
                Arena::Ptr<Destructive> p0 {Arena::Ptr<Destructive>::simpleDeleter, &arena, "p0"};

                p0->sayHi();
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
