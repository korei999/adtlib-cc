#pragma once

#include "adt/types.hh"

#if __has_include(<pthread.h>)
    #include <pthread.h>
    #define ADT_USE_PTHREAD
#elif __has_include(<windows.h>)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN 1
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #define ADT_USE_WIN32THREAD
#endif

namespace adt
{

using THREAD_STATUS = usize;

struct Thread
{
#ifdef ADT_USE_PTHREAD
    pthread_t m_thread {};
#elif defined ADT_USE_WIN32THREAD
    HANDLE m_thread {};
    DWORD m_id {};
#else
    #error "No platform threads"
#endif

    /* */

    Thread() = default;
    Thread(THREAD_STATUS (*pfn)(void*), void* pFnArg);

    /* */

    THREAD_STATUS join();
    THREAD_STATUS detach();

private:
#ifdef ADT_USE_PTHREAD

    THREAD_STATUS pthreadJoin();
    THREAD_STATUS pthreadDetach();

#elif defined ADT_USE_WIN32THREAD

    THREAD_STATUS win32Join();
    THREAD_STATUS win32Detach();

#endif
};

inline
Thread::Thread(THREAD_STATUS (*pfn)(void*), void* pFnArg)
{
#ifdef ADT_USE_PTHREAD
    pthread_create(&m_thread, {}, (void* (*)(void*))pfn, pFnArg);
#elif defined ADT_USE_WIN32THREAD
    m_thread = CreateThread(nullptr, 0, (DWORD (*)(void*))pfn, pFnArg, 0, &m_id);
#endif
}

inline THREAD_STATUS
Thread::join()
{
#ifdef ADT_USE_PTHREAD
    return pthreadJoin();
#elif defined ADT_USE_WIN32THREAD
    return win32Join();
#endif
}

inline THREAD_STATUS
Thread::detach()
{
#ifdef ADT_USE_PTHREAD
    return pthreadDetach();
#elif defined ADT_USE_WIN32THREAD
    return win32Detach();
#endif
}


#ifdef ADT_USE_PTHREAD

inline THREAD_STATUS
Thread::pthreadJoin()
{
    void* pRet {};
    pthread_join(m_thread, &pRet);

    return (THREAD_STATUS)pRet;
}

inline THREAD_STATUS
Thread::pthreadDetach()
{
    return (THREAD_STATUS)pthread_detach(m_thread);
}

#elif defined ADT_USE_WIN32THREAD

inline THREAD_STATUS
Thread::win32Join()
{
    return WaitForSingleObject(m_thread, INFINITE);
}

inline THREAD_STATUS
Thread::win32Detach()
{
    return {};
}

#endif

} /* namespace adt */
