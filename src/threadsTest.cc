#include "adt/logs.hh"
#include "adt/print.hh"
#include "adt/Thread.hh"
#include "adt/defer.hh"

using namespace adt;

static THREAD_STATUS
thread(void* pArg)
{
    if (pArg)
    {
        print::out("pArg: {}\n", *(int*)pArg);
        return *(THREAD_STATUS*)pArg;
    }
    else
    {
        print::out("pArg: {}\n", nullptr);
        return 0;
    }
}

int
main()
{
    int one = 1;

    {
        Thread thrd1(thread, &one);
        defer( thrd1.join() );

        Thread thrd2(thread, {});
        defer( thrd2.join() );
    }

    LOG("one: {}\n", one);
}
