#include "adt/logs.hh"
#include "adt/defer.hh"
#include "adt/Arena.hh"
#include "adt/Queue.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("Queue test...\n");

    Arena arena(SIZE_1K);
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
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 2, "expected: 2, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 3, "expected: 3, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 4, "expected: 4, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 5, "expected: 5, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 6, "expected: 6, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 7, "expected: 7, got: {}", x);
            LOG("x: {}\n", x);
        }

        {
            int x = q.popFront();
            ADT_ASSERT_ALWAYS(x == 8, "expected: 8, got: {}", x);
            LOG("x: {}\n", x);
        }

        ADT_ASSERT_ALWAYS(q.empty(), "must be empty, size: {}", q.size());

        for (const auto& e : q)
            LOG("(#{}): {}\n", q.idx(&e), e);

        CERR("\n");
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
            LOG("(#{}): {}\n", q.idx(&e), e);

        CERR("\n");
    }

    LOG_GOOD("Queue test passed\n");
}
