#include "adt/print.hh"
#include "adt/Thread.hh"

using namespace adt;

static usize
thread(void* pArg)
{
    if (pArg)
        print::out("pArg: {}\n", *(int*)pArg);
    else
        print::out("pArg: {}\n", nullptr);

    return 0;
}

int
main()
{
    int one = 1;
    Thread thrd1(thread, &one);
    thrd1.join();

    Thread thrd2(thread, {});
    thrd2.join();
}
