#pragma once

#include "FuncBuffer.hh"
#include "QueueArray.hh"
#include "ScratchBuffer.hh"
#include "StdAllocator.hh"
#include "Thread.hh"
#include "Vec.hh"
#include "atomic.hh"
#include "defer.hh"

namespace adt
{

struct IThreadPool
{
    using Task = FuncBuffer<void, 56>; /* 64 bytes */

    struct Future
    {
        IThreadPool* m_pPool {};
        Mutex m_mtx {};
        CndVar m_cnd {};
        bool m_bDone {};

        /* */

        Future() noexcept = default;
        Future(IThreadPool* pPool) noexcept;

        /* */

        void wait() noexcept;
        void signal() noexcept;
    };

    /* */

    static int
    optimalThreadCount() noexcept
    {
        static const int s_count = utils::max(ADT_GET_NPROCS() - 1, 1);
        return s_count;
    }

    /* */

    virtual const atomic::Int& nActiveTasks() const noexcept = 0;

    virtual int nThreads() const noexcept = 0;

    virtual bool addTask(void (*pfn)(void*), void* pArg, isize argSize) noexcept = 0;

    virtual void wait() noexcept = 0;

    virtual Task tryStealTask() noexcept = 0;

    template<typename CL>
    bool
    add(CL& cl) noexcept
    {
        return addTask([](void* p) {
            static_cast<CL*>(p)->operator()();
        }, (void*)&cl, sizeof(cl));
    }

    template<typename CL>
    bool
    add(Future* pFut, CL& cl) noexcept
    {
        auto cl2 = [pFut, cl]
        {
            cl();
            pFut->signal();
        };

        static_assert(sizeof(cl2) >= sizeof(cl) + sizeof(pFut));

        return addTask([](void* p) {
            static_cast<decltype(cl2)*>(p)->operator()();
        }, &cl2, sizeof(cl2));
    }

    template<typename CL>
    void addRetry(const CL& cl) noexcept { while (!add(cl)); }

    template<typename CL>
    void addRetry(Future* pFut, const CL& cl) noexcept { while (!add(pFut, cl)); }

    template<typename CL>
    void
    addRetryOrDo(CL& cl) noexcept
    {
        if (nActiveTasks().load(atomic::ORDER::ACQUIRE) >= nThreads()) cl();
        else addRetry(cl);
    }
};

inline
IThreadPool::Future::Future(IThreadPool* pPool) noexcept
    : m_pPool {pPool}, m_mtx {INIT}, m_cnd {INIT}, m_bDone {false}
{
    ADT_ASSERT(pPool != nullptr, "");
}

inline void
IThreadPool::Future::wait() noexcept
{
    Task task;
    while ((task = m_pPool->tryStealTask()))
        task();

    LockGuard lock {&m_mtx};
    while (!m_bDone) m_cnd.wait(&m_mtx);
}

inline void
IThreadPool::Future::signal() noexcept
{
    {
        LockGuard lock {&m_mtx};
        m_bDone = true;
    }
    m_cnd.signal();
}

template<isize QUEUE_SIZE>
struct ThreadPool : IThreadPool
{
    Span<Thread> m_spThreads {};
    Mutex m_mtxQ {};
    CndVar m_cndQ {};
    CndVar m_cndWait {};
    void (*m_pfnLoopStart)(void*) {};
    void* m_pLoopStartArg {};
    void (*m_pfnLoopEnd)(void*) {};
    void* m_pLoopEndArg {};
    atomic::Int m_atomNActiveTasks {};
    atomic::Int m_atomBDone {};
    atomic::Int m_atomIdCounter {};
    atomic::Int m_atomBPollMode {};
    bool m_bStarted {};
    QueueArray<Task, QUEUE_SIZE> m_qTasks {};

    /* */

    static inline thread_local int gtl_threadId {};

    /* */

    ThreadPool() = default;

    ThreadPool(IAllocator* pAlloc, int nThreads = optimalThreadCount());

    ThreadPool(
        IAllocator* pAlloc,
        void (*pfnOnLoopStart)(void*),
        void* pLoopStartArg,
        void (*pfnOnLoopEnd)(void*),
        void* pLoopEndArg,
        int nThreads = optimalThreadCount()
    );

    /* */

    virtual const atomic::Int& nActiveTasks() const noexcept override { return m_atomNActiveTasks; }

    virtual void wait() noexcept override;

    void destroy(IAllocator* pAlloc) noexcept;

    virtual bool addTask(void (*pfn)(void*), void* pArg, isize argSize) noexcept override;

    virtual int nThreads() const noexcept override { return m_spThreads.size(); }

    virtual Task tryStealTask() noexcept override;

    /* */

    void enablePollMode() noexcept;
    void disablePollMode() noexcept;

protected:
    void start();
    THREAD_STATUS loop();
};

template<isize QUEUE_SIZE>
inline
ThreadPool<QUEUE_SIZE>::ThreadPool(IAllocator* pAlloc, int nThreads)
    : m_spThreads(pAlloc->zallocV<Thread>(nThreads), nThreads),
      m_mtxQ(Mutex::TYPE::PLAIN),
      m_cndQ(INIT),
      m_cndWait(INIT)
{
    start();
}

template<isize QUEUE_SIZE>
inline
ThreadPool<QUEUE_SIZE>::ThreadPool(
    IAllocator* pAlloc,
    void (*pfnOnLoopStart)(void*), void* pLoopStartArg,
    void (*pfnOnLoopEnd)(void*), void* pLoopEndArg,
    int nThreads
)
    : m_spThreads(pAlloc->zallocV<Thread>(nThreads), nThreads),
      m_mtxQ(Mutex::TYPE::PLAIN),
      m_cndQ(INIT),
      m_cndWait(INIT),
      m_pfnLoopStart(pfnOnLoopStart),
      m_pLoopStartArg(pLoopStartArg),
      m_pfnLoopEnd(pfnOnLoopEnd),
      m_pLoopEndArg(pLoopEndArg)
{
    start();
}

template<isize QUEUE_SIZE>
inline THREAD_STATUS
ThreadPool<QUEUE_SIZE>::loop()
{
    if (m_pfnLoopStart) m_pfnLoopStart(m_pLoopStartArg);
    ADT_DEFER( if (m_pfnLoopEnd) m_pfnLoopEnd(m_pLoopEndArg) );

    gtl_threadId = 1 + m_atomIdCounter.fetchAdd(1, atomic::ORDER::RELAXED);

    while (true)
    {
        Task task {};

        {
            LockGuard qLock {&m_mtxQ};

            while (m_atomBPollMode.load(atomic::ORDER::ACQUIRE) == false &&
                m_qTasks.empty() &&
                !m_atomBDone.load(atomic::ORDER::ACQUIRE)
            )
            {
                m_cndQ.wait(&m_mtxQ);
            }

            if (m_atomBDone.load(atomic::ORDER::ACQUIRE))
                return 0;

            task = m_qTasks.popFront();
            m_atomNActiveTasks.fetchAdd(1, atomic::ORDER::RELAXED);
        }

        if (task) task();
        m_atomNActiveTasks.fetchSub(1, atomic::ORDER::RELEASE);

        {
            LockGuard qLock {&m_mtxQ};

            if (m_qTasks.empty() && m_atomNActiveTasks.load(atomic::ORDER::ACQUIRE) <= 0)
                m_cndWait.signal();
        }
    }

    return THREAD_STATUS(0);
}

template<isize QUEUE_SIZE>
inline void
ThreadPool<QUEUE_SIZE>::start()
{
    for (auto& thread : m_spThreads)
    {
        thread = Thread(
            reinterpret_cast<ThreadFn>(methodPointerNonVirtual(&ThreadPool::loop)),
            this
        );
    }

    m_bStarted = true;

#ifndef NDEBUG
    print::err("[ThreadPool]: new pool with {} threads\n", m_spThreads.size());
#endif
}

template<isize QUEUE_SIZE>
inline void
ThreadPool<QUEUE_SIZE>::wait() noexcept
{
again:
    m_mtxQ.lock();
    if (!m_qTasks.empty())
    {
        Task task = m_qTasks.popFront();
        m_mtxQ.unlock();

        ADT_ASSERT(task, "");
        if (task) task();

        goto again;
    }
    else
    {
        m_mtxQ.unlock();
    }

    LockGuard qLock {&m_mtxQ};

    while (!m_qTasks.empty() || m_atomNActiveTasks.load(atomic::ORDER::RELAXED) > 0)
        m_cndWait.wait(&m_mtxQ);
}

template<isize QUEUE_SIZE>
inline void
ThreadPool<QUEUE_SIZE>::destroy(IAllocator* pAlloc) noexcept
{
    wait();

    {
        LockGuard qLock {&m_mtxQ};
        m_atomBDone.store(true, atomic::ORDER::RELEASE);
    }

    m_cndQ.broadcast();

    for (auto& thread : m_spThreads)
        thread.join();

    ADT_ASSERT(m_atomNActiveTasks.load(atomic::ORDER::ACQUIRE) == 0, "{}", m_atomNActiveTasks.load(atomic::ORDER::RELAXED));

    pAlloc->free(m_spThreads.data());
    m_mtxQ.destroy();
    m_cndQ.destroy();
    m_cndWait.destroy();
}

template<isize QUEUE_SIZE>
inline bool
ThreadPool<QUEUE_SIZE>::addTask(void (*pfn)(void*), void* pArg, isize argSize) noexcept
{
    ADT_ASSERT(m_bStarted, "forgot to `start()` this ThreadPool: (m_bStarted: '{}')", m_bStarted);

    isize i;
    {
        LockGuard lock {&m_mtxQ};
        i = m_qTasks.emplaceBack(pfn, pArg, argSize);
    }

    if (i != -1)
    {
        m_cndQ.signal();
        return true;
    }

    return false;
}

template<isize QUEUE_SIZE>
inline IThreadPool::Task
ThreadPool<QUEUE_SIZE>::tryStealTask() noexcept
{
    m_mtxQ.lock();

    if (!m_qTasks.empty())
    {
        Task task = m_qTasks.popFront();
        m_mtxQ.unlock();
        return task;
    }
    else
    {
        m_mtxQ.unlock();
        return {};
    }
}

template<isize QUEUE_SIZE>
inline void
ThreadPool<QUEUE_SIZE>::enablePollMode() noexcept
{
    m_atomBPollMode.store(true, atomic::ORDER::RELEASE);
}

template<isize QUEUE_SIZE>
inline void
ThreadPool<QUEUE_SIZE>::disablePollMode() noexcept
{
    m_atomBPollMode.store(false, atomic::ORDER::RELEASE);
}

struct IThreadPoolWithMemory : IThreadPool
{
    virtual ScratchBuffer& scratchBuffer() = 0;
};

/* ThreadPool with ScratchBuffers created for each thread.
 * Any thread can access its own thread local buffer with `threadPool.scratch()`. */
template<isize QUEUE_SIZE>
struct ThreadPoolWithMemory : IThreadPoolWithMemory
{
    using Task = ThreadPool<QUEUE_SIZE>::Task;

    /* */

    static inline thread_local u8* gtl_pScratchMem;
    static inline thread_local ScratchBuffer gtl_scratchBuff;

    /* */
    ThreadPool<QUEUE_SIZE> m_base {};

    /* */

    ThreadPoolWithMemory() = default;

    ThreadPoolWithMemory(IAllocator* pAlloc, isize nBytesEachBuffer, int nThreads = optimalThreadCount())
        : m_base(
            pAlloc,
            +[](void* p) { allocScratchBufferForThisThread(reinterpret_cast<isize>(p)); },
            reinterpret_cast<void*>(nBytesEachBuffer),
            +[](void*) { destroyScratchBufferForThisThread(); },
            nullptr,
            nThreads
        )
    {
        ADT_ASSERT(nThreads > 0, "nThreads: {}", nThreads);
        allocScratchBufferForThisThread(nBytesEachBuffer);
    }

    /* */

    virtual ScratchBuffer& scratchBuffer() override { return gtl_scratchBuff; }
    virtual const atomic::Int& nActiveTasks() const noexcept override { return m_base.nActiveTasks(); }
    virtual void wait() noexcept override { m_base.wait(); }
    virtual bool addTask(void (*pfn)(void*), void* pArg, isize argSize) noexcept override { return m_base.addTask(pfn, pArg, argSize); }
    virtual int nThreads() const noexcept override { return m_base.nThreads(); }
    virtual Task tryStealTask() noexcept override { return m_base.tryStealTask(); }

    /* */

    void
    destroy(IAllocator* pAlloc) noexcept
    {
        m_base.destroy(pAlloc);
        destroyScratchBufferForThisThread();
    }

    /* `destroyScratchBufferForThisThread()` later. */
    void
    destroyKeepScratchBuffer(IAllocator* pAlloc) noexcept
    {
        m_base.destroy(pAlloc);
    }

    /* */

    static void
    allocScratchBufferForThisThread(isize size)
    {
        ADT_ASSERT(gtl_pScratchMem == nullptr, "already allocated");

        gtl_pScratchMem = StdAllocator::inst()->zallocV<u8>(size);
        gtl_scratchBuff = ScratchBuffer {gtl_pScratchMem, size};
    }

    static void
    destroyScratchBufferForThisThread() noexcept
    {
        StdAllocator::inst()->free(gtl_pScratchMem);
        gtl_pScratchMem = {};
        gtl_scratchBuff = {};
    }
};

/* Usage example:
 * Vec<Future<Span<f32>>*> vFutures = parallelFor(&arena, &tp, Span<f32> {v},
 *     [](Span<f32> spBatch, isize offFrom0)
 *     {
 *         for (auto& e : spBatch) r += 1 + offFrom0;
 *     }
 * );
 * 
 *  for (auto* pF : vFutures) pF->wait(); */
template<typename THREAD_POOL_T, typename T, typename CL_PROC_BATCH>
[[nodiscard]] inline Vec<Future<Span<T>>*>
parallelFor(IArena* pArena, THREAD_POOL_T* pTp, Span<T> spData, CL_PROC_BATCH clProcBatch, isize minBatchSize = 1)
{
    if (spData.size() < 0) return {};

    const isize nThreads = pTp->nThreads();
    const isize batchSize = [&]
    {
        const isize len = spData.size() / nThreads;
        const isize div = len > 0 ? len : spData.size();
        return div < minBatchSize ? minBatchSize : div;
    }();
    const isize tailSize = spData.size() - nThreads*batchSize;

    struct Arg
    {
        Future<Span<T>> future {};
        Span<T> sp {};
        isize off {};
        decltype(clProcBatch) cl {};
    };

    Vec<Future<Span<T>>*> vFutures {pArena, tailSize > 0 ? nThreads + 1 : nThreads};

    auto clBatch = [&](isize off, isize size) {
        Arg* pArg = pArena->alloc<Arg>(INIT, Span<T> {spData.data() + off, size}, off, clProcBatch);
        pArg->future.data() = pArg->sp;

        vFutures.push(pArena, &pArg->future);

        pTp->addRetry(
            +[](void* p) -> THREAD_STATUS
            {
                Arg& arg = *static_cast<Arg*>(p);
                arg.cl(arg.sp, arg.off);
                arg.future.signal();
                return THREAD_STATUS(0);
            },
            pArg
        );
    };

    isize i = 0;
    for (; i < batchSize*nThreads; i += batchSize) clBatch(i, batchSize);
    if (tailSize > 0) clBatch(i, tailSize);

    return vFutures;
}

} /* namespace adt */
