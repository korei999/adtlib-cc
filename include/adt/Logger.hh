#pragma once

#include "Queue.hh"
#include "Thread.hh"

namespace adt
{

struct ILogger
{
    enum class LEVEL : i8
    {
        NONE = -1,
        ERROR = 0,
        WARN,
        INFO,
        DEBUG,
    };

    /* */

    LEVEL m_eLevel = LEVEL::WARN;

    /* */

    ILogger() noexcept = default;
    ILogger(LEVEL eLevel) noexcept : m_eLevel {eLevel} {}

    /* */

    virtual void pushString(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept = 0;
};

template<isize SIZE = 512, typename ...ARGS>
struct Log
{    
    Log(ILogger::LEVEL eLevel, ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
    {
        ADT_ASSERT(eLevel >= ILogger::LEVEL::NONE && eLevel <= ILogger::LEVEL::DEBUG,
            "eLevel: {}, (min: {}, max: {})", (int)eLevel, (int)ILogger::LEVEL::NONE, (int)ILogger::LEVEL::DEBUG
        );
        if (eLevel > pLogger->m_eLevel) return;

        StringFixed<SIZE> msg;
        isize n = 0;
        // n += print::toBuffer(msg.data() + n, msg.cap() - n, "({}, {}): ", print::stripSourcePath(loc.file_name()), loc.line());
        n += print::toBuffer(msg.data() + n, msg.cap() - n, std::forward<ARGS>(args)...);
        pLogger->pushString(eLevel, loc, StringView{msg.data(), n});
    }
};

template<isize SIZE = 512, typename ...ARGS>
struct LogError : Log<SIZE, ARGS...>
{
    LogError(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...> {ILogger::LEVEL::ERROR, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogWarn : Log<SIZE, ARGS...>
{
    LogWarn(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...> {ILogger::LEVEL::WARN, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogInfo : Log<SIZE, ARGS...>
{
    LogInfo(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...> {ILogger::LEVEL::INFO, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogDebug : Log<SIZE, ARGS...>
{
    LogDebug(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...> {ILogger::LEVEL::DEBUG, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
Log(ILogger::LEVEL eLevel, ILogger*, ARGS&&...) -> Log<SIZE, ARGS...>;

template<isize SIZE = 512, typename ...ARGS>
LogError(ILogger*, ARGS&&...) -> LogError<SIZE, ARGS...>;

template<isize SIZE = 512, typename ...ARGS>
LogWarn(ILogger*, ARGS&&...) -> LogWarn<SIZE, ARGS...>;

template<isize SIZE = 512, typename ...ARGS>
LogInfo(ILogger*, ARGS&&...) -> LogInfo<SIZE, ARGS...>;

template<isize SIZE = 512, typename ...ARGS>
LogDebug(ILogger*, ARGS&&...) -> LogDebug<SIZE, ARGS...>;

struct Logger : ILogger
{
    struct Msg
    {
        StringFixed<512> sf;
        isize msgSize;
        LEVEL eLevel;
        std::source_location loc;
    };

    /* */

    QueueM<Msg> m_q;
    Mutex m_mtxQ;
    CndVar m_cnd;
    bool m_bDone;
    FILE* m_pFile;
    Thread m_thrd;

    /* */

    Logger(ILogger::LEVEL eLevel, FILE* pFile, isize maxQueueSize);
    Logger() noexcept : m_q {}, m_mtxQ {}, m_cnd {}, m_bDone {}, m_pFile {}, m_thrd {} {}
    Logger(UninitFlag) noexcept {}

    /* */

    virtual void pushString(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept override;

    /* */

    void destroy() noexcept;

protected:
    THREAD_STATUS loop() noexcept;
};

inline
Logger::Logger(ILogger::LEVEL eLevel, FILE* pFile, isize maxQueueSize)
    : ILogger {eLevel},
      m_q {maxQueueSize},
      m_mtxQ {Mutex::TYPE::PLAIN},
      m_cnd {INIT},
      m_bDone {false},
      m_pFile {pFile},
      m_thrd {(ThreadFn)methodPointerNonVirtual(&Logger::loop), this}
{
}

inline THREAD_STATUS
Logger::loop() noexcept
{
    char aBuff[256] {};

    while (true)
    {
        Msg msg;
        {
            LockGuard lock {&m_mtxQ};
            while (!m_bDone && m_q.empty())
                m_cnd.wait(&m_mtxQ);

            if (m_bDone && m_q.empty()) break;

            msg = m_q.popFront();
        }

        isize n = 0;
        switch (msg.eLevel)
        {
            case LEVEL::NONE:
            {
            }
            break;

            case LEVEL::ERROR:
            {
            }
            break;

            case LEVEL::WARN:
            {
            }
            break;

            case LEVEL::INFO:
            {
            }
            break;

            case LEVEL::DEBUG:
            n = print::toSpan(aBuff, "(DEBUG: {}, {}): ", print::stripSourcePath(msg.loc.file_name()), msg.loc.line());
            break;
        }

        fwrite(aBuff, n, 1, m_pFile);
        fwrite(msg.sf.data(), msg.msgSize, 1, m_pFile);
    }

    return THREAD_STATUS(0);
}

inline void
Logger::pushString(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept
{
    {
        LockGuard lock {&m_mtxQ};
        m_q.emplaceBackNoGrow(sv, sv.size(), eLevel, loc);
    }
    m_cnd.signal();
}

inline void
Logger::destroy() noexcept
{
    {
        LockGuard lock {&m_mtxQ};
        m_bDone = true;
        m_cnd.signal();
    }

    m_thrd.join();

    m_q.destroy();
    m_mtxQ.destroy();
    m_cnd.destroy();
}

} /* namespace adt */
