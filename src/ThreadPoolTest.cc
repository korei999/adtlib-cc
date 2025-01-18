#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/ThreadPool.hh"
#include "adt/BufferAllocator.hh"
#include "adt/ScratchBuffer.hh"

using namespace adt;

thread_local static u8 tls_aBuff[2048] {};
thread_local static ScratchBuffer tls_Scratch(Span{tls_aBuff});

static int
task(void* pArg)
{
    int size = 2048 / 4;
    auto spBuff = tls_Scratch.getZMem<char>(size);
    print::toBuffer(spBuff.data(), spBuff.getSize() - 1, "must not corrupt this string");
    utils::sleepMS(*(f64*)pArg);

    assert(String(spBuff.data()) == "must not corrupt this string");

    LOG("done waiting for {}\n", *(f64*)pArg);
    return thrd_success;
}

static int
signaled(void*)
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
    tp.submitSignal(signaled, {}, &lock);
    tp.submit(task, &ms);

    for (ssize i = 0; i < 100; ++i)
        tp.submit(task, &ms);

    tp.wait();

    lock.wait();
}
