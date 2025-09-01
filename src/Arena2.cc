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
            {
                struct Destructive
                {
                    int m_i;
                    Destructive() noexcept { m_i = ++i; LogDebug{"m_i: {}\n", m_i}; }

                    ~Destructive() noexcept
                    {
                        LogDebug{"{} dies...\n", m_i};
                        --i;
                    };

                    void sayHi() noexcept { LogDebug{"{} says hi\n", m_i}; }
                };

                Arena::Owned<Destructive> pp0 = arena.allocOwnedWithDeleter<Destructive>([](Arena*, void* pD) {
                    auto& r = (*(Destructive*)pD);
                    LogWarn("({}) custom deleter in top frame\n", r.m_i);
                    r.~Destructive();
                });
                pp0->sayHi();

                LogWarn("offset before push: {}\n", arena.m_off);
                ArenaStateGuard pushed {&arena};

                Arena::Owned<Destructive> pp1 = arena.allocOwned<Destructive>();
                Arena::Owned<Destructive> pp2 = arena.allocOwned<Destructive>();
                Arena::Owned<Destructive> pp3 = arena.allocOwnedWithDeleter<Destructive>([](Arena*, void* pD) {
                    auto& r = (*(Destructive*)pD);
                    LogDebug("({}) custom deleter\n", r.m_i);
                    r.~Destructive();
                });

                pp1->sayHi();
                pp2->sayHi();
                pp3->sayHi();
            }
            LogDebug("offset after pop: {}\n", arena.m_off);

            ADT_ASSERT_ALWAYS(i == 1, "i: {}\n", i);
        }
    }
    catch (const IException& ex)
    {
        LogDebug("{}\n", ex.getMsg());
    }

    LogInfo{"Arena2 test passed\n"};
}
