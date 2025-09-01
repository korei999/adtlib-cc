#include "adt/Arena.hh"
#include "adt/Logger.hh"
#include "adt/Vec.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Arena2 test...\n"};

    try
    {
        Arena arena {SIZE_8G, 0};
        defer( arena.freeAll() );

        {
            ArenaStateGuard pushedTop {&arena};

            Vec<isize> v0 {};
            Vec<isize> v1 {};

            for (isize i = 0; i < 100; ++i)
            {
                if (i & 1) v0.push(&arena, i);
                else v1.push(&arena, i);
            }

            for (auto e : v0) ADT_ASSERT_ALWAYS(e & 1, "e: {}", e);
            for (auto e : v1) ADT_ASSERT_ALWAYS(!(e & 1), "e: {}", e);

            {
                ArenaStateGuard pushed {&arena};
                print::toFILE(&arena, stdout, "v0: {}\n", v0);
                LogInfo("off before pop: {}\n", arena.m_off);
            }

            {
                ArenaStateGuard pushed {&arena};
                print::toFILE(&arena, stdout, "v1: {}\n", v1);
                LogInfo("off before pop: {}\n", arena.m_off);
            }
        }

        {
            static int i = 0;

            struct Destructive
            {
                int m_i;
                const StringView m_sv;

                Destructive(const StringView sv) noexcept : m_sv{sv} { m_i = ++i; LogDebug{"({}) m_i: {}\n", sv, m_i}; }

                ~Destructive() noexcept
                {
                    LogDebug{"({}) {} dies...\n", m_sv, m_i};
                    --i;
                };

                void sayHi() noexcept { LogDebug{"{} says hi\n", m_i}; }
            };

            Arena::Ptr<Destructive> pp;

            {
                ArenaStateGuard pushed {&arena};

                new(&pp) Arena::Ptr<Destructive> {&arena, "pp"};

                Arena::Ptr<Destructive> pp0 {&arena, "pp0"};
                Arena::Ptr<Destructive> pp1 {&arena, "pp1"};
                Arena::Ptr<Destructive> pp2 {&arena, "pp2"};

                pp0->sayHi();
                pp1->sayHi();
                pp2->sayHi();
            }
            LogDebug("offset after pop: {}\n", arena.m_off);

            Arena::Ptr<Destructive> pp3 {[](Arena*, void** ppObj) {
                ((Destructive*)*ppObj)->~Destructive();
                *((Destructive**)ppObj) = nullptr;
            }, &arena, "pp3"};

            pp3->sayHi();

            ADT_ASSERT_ALWAYS(!pp, "!pp: {}", !pp);
            if (pp) pp->sayHi();

            ADT_ASSERT_ALWAYS(i == 1, "i: {}", i);

            arena.reset();
            ADT_ASSERT_ALWAYS(!pp3, "!pp: {}", !pp3);
        }
    }
    catch (const IException& ex)
    {
        LogDebug("{}\n", ex.getMsg());
    }

    LogInfo{"Arena2 test passed\n"};
}
