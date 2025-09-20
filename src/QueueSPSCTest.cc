#include "adt/Thread.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Logger.hh"

#include "WIP/QueueSPSC.hh"

using namespace adt;

static QueueSPSC<int, 64> s_q;

static THREAD_STATUS
popTheQ(void*)
{
    Opt<int> i {};

    int counter = 0;

    do
    {
        i = s_q.popFront();
        if (i)
        {
            if (i.value() == -1) break;

            ADT_ASSERT_ALWAYS(i.value() == counter, "");
            LOG_GOOD("i: {}\n", i.value());
            ++counter;
        }
        else
        {
            Thread::yield();
        }
    }
    while (true);

    return THREAD_STATUS(0);
}

int
main()
{
    LOG_NOTIFY("QueueSPSCTest...\n");

    Thread consumer {popTheQ, nullptr};

    for (int i = 0; i < 1000000; ++i)
    {
        while (!s_q.pushBack(i))
            Thread::yield();
    }

    while (!s_q.pushBack(-1))
        ;

    consumer.join();
}
