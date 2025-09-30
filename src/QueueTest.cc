#include "adt/QueueArray.hh"
#include "adt/defer.hh"
#include "adt/ArenaList.hh"
#include "adt/Queue.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo("Queue test...\n");

    ArenaList arena(SIZE_1K);
    defer( arena.freeAll() );

    {
        Queue<int> q {&arena};

        q.pushBack(&arena, 1);
        q.pushBack(&arena, 2);
        q.pushBack(&arena, 3);
        q.pushBack(&arena, 4);
        q.pushBack(&arena, 5);
        q.pushBack(&arena, 6);
        q.pushBack(&arena, 7);
        q.pushBack(&arena, 8);

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 1, "expected: 1, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 2, "expected: 2, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 3, "expected: 3, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 4, "expected: 4, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 5, "expected: 5, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 6, "expected: 6, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 7, "expected: 7, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 8, "expected: 8, got: {}", x);
            LogDebug("x: {}\n", x);
        }

        ADT_ASSERT_ALWAYS(q.empty(), "must be empty, size: {}", q.size());

        for (const auto& e : q)
            print::err("(#{}): {}\n", q.idx(&e), e);
        print::err("\n");
    }

    {
        Queue<int> q {&arena, 4};

        q.pushBack(&arena, 1);
        q.pushBack(&arena, 2);
        q.pushBack(&arena, 3);
        q.pushBack(&arena, 4);

        {
            auto x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 1, "expected: 1, got: {}", x);
        }

        q.pushBack(&arena, 5);
        q.pushBack(&arena, 6);

        {
            auto x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 2, "expected: 2, got: {}", x);
        }

        {
            auto x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 3, "expected: 3, got: {}", x);
        }

        q.pushBack(&arena, 7);
        q.pushBack(&arena, 8);

        q.pushFront(&arena, 0);

        {
            auto x = q.popBack();
            ADT_ASSERT_ALWAYS(x == 8, "expected: 8, got: {}", x);
        }

        for (const auto& e : q)
            print::err("(#{}): {}\n", q.idx(&e), e);
        print::err("\n");
    }

    {
        QueueManaged<int> qq {10};
        auto qq2 = qq.release();
        defer( qq2.destroy() );
    }

    {
        QueueArray<int, 16> q0;

        q0.pushBack(0);
        q0.pushBack(1);
        q0.pushBack(2);
        q0.pushBack(3);

        q0.pushFront(4);
        q0.pushFront(5);
        q0.pushFront(6);

        LogDebug("q0: {}\n", q0);
    }

    LogInfo("Queue test passed\n");
}
