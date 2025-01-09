#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"

using namespace adt;

static int
task(void* pArg)
{
    utils::sleepMS(*(f64*)pArg);

    LOG("done waiting for {}\n", *(f64*)pArg);
    return thrd_success;
}

static int
signaled(void* pArg)
{
    LOG("signaled\n");

    return thrd_success;
}

static u8 s_aBuff[100000] {};

int
main()
{
    BufferAllocator fixed(s_aBuff, sizeof(s_aBuff));

    ThreadPool tp(&fixed);
    defer( tp.destroy() );
    tp.start();

    f64 ms = 500.0;

    ThreadPoolLock lock(INIT);
    defer( lock.destroy() );

    tp.submit(task, &ms);
    tp.submit(task, &ms);
    tp.submitSignal(signaled, {}, &lock);
    tp.submit(task, &ms);

    tp.wait();

    lock.wait();
}
