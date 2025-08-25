#pragma once

#include "print.hh"

#define ADT_LOGGER_COL_NORM  "\x1b[0m"
#define ADT_LOGGER_COL_RED  "\x1b[31m"
#define ADT_LOGGER_COL_GREEN  "\x1b[32m"
#define ADT_LOGGER_COL_YELLOW  "\x1b[33m"
#define ADT_LOGGER_COL_BLUE  "\x1b[34m"
#define ADT_LOGGER_COL_MAGENTA  "\x1b[35m"
#define ADT_LOGGER_COL_CYAN  "\x1b[36m"
#define ADT_LOGGER_COL_WHITE  "\x1b[37m"

namespace adt
{

struct ILogger
{
    enum class LEVEL : i8 {NONE = -1, ERROR = 0, WARN, INFO, DEBUG};

    /* */

    LEVEL m_eLevel = LEVEL::WARN;
    bool m_bTTY = false;

    /* */

    ILogger() noexcept = default;
    ILogger(LEVEL eLevel, FILE* pFile) noexcept : m_eLevel {eLevel}, m_bTTY {isTTY(pFile)} {}

    /* */

    virtual void add(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept = 0;
    virtual isize formatHeader(LEVEL eLevel, std::source_location loc, Span<char> spBuff) noexcept = 0;

    /* */

    static bool isTTY(FILE* pFile) noexcept;
};

namespace print
{

inline isize
format(Context ctx, FormatArgs fmtArgs, const ILogger::LEVEL& x)
{
    constexpr StringView mapStrings[] {"", "ERROR", "WARN", "INFO", "DEBUG"};
    ADT_ASSERT((int)x + 1 >= 0 && (int)x + 1 < utils::size(mapStrings), "{}", (int)x);
    return format(ctx, fmtArgs, mapStrings[(int)x + 1]);
}

} /* namespace print */

} /* namespace adt */

#include "Queue.hh"
#include "Thread.hh"

namespace adt
{

inline bool
ILogger::isTTY(FILE* pFile) noexcept
{
#ifdef _MSC_VER
    return _isatty(_fileno(pFile));
#else
    return isatty(fileno(pFile));
#endif
}

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
        isize n = print::toSpan(msg.data(), std::forward<ARGS>(args)...);
        pLogger->add(eLevel, loc, StringView{msg.data(), n});
    }
};

template<isize SIZE = 512, typename ...ARGS>
struct LogError : Log<SIZE, ARGS...>
{
    LogError(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...>{ILogger::LEVEL::ERROR, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogWarn : Log<SIZE, ARGS...>
{
    LogWarn(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...>{ILogger::LEVEL::WARN, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogInfo : Log<SIZE, ARGS...>
{
    LogInfo(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...>{ILogger::LEVEL::INFO, pLogger, std::forward<ARGS>(args)..., loc} {}
};

template<isize SIZE = 512, typename ...ARGS>
struct LogDebug : Log<SIZE, ARGS...>
{
    LogDebug(ILogger* pLogger, ARGS&&... args, const std::source_location& loc = std::source_location::current())
        : Log<SIZE, ARGS...>{ILogger::LEVEL::DEBUG, pLogger, std::forward<ARGS>(args)..., loc} {}
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

    virtual void add(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept override;
    virtual isize formatHeader(LEVEL eLevel, std::source_location loc, Span<char> spBuff) noexcept override;

    /* */

    void destroy() noexcept;

protected:
    THREAD_STATUS loop() noexcept;
};

inline
Logger::Logger(ILogger::LEVEL eLevel, FILE* pFile, isize maxQueueSize)
    : ILogger {eLevel, pFile},
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

        isize n = formatHeader(msg.eLevel, msg.loc, aBuff);

        fwrite(aBuff, n, 1, m_pFile);
        fwrite(msg.sf.data(), msg.msgSize, 1, m_pFile);
    }

    return THREAD_STATUS(0);
}

inline void
Logger::add(LEVEL eLevel, std::source_location loc, const StringView sv) noexcept
{
    {
        LockGuard lock {&m_mtxQ};
        m_q.emplaceBackNoGrow(sv, sv.size(), eLevel, loc);
    }
    m_cnd.signal();
}

inline isize
Logger::formatHeader(LEVEL eLevel, std::source_location loc, Span<char> spBuff) noexcept
{
    if (eLevel == LEVEL::NONE) return 0;

    StringView svCol0 {};
    StringView svCol1 {};

    if (m_bTTY)
    {
        switch (eLevel)
        {
            case LEVEL::NONE:
            return 0;

            case LEVEL::ERROR:
            svCol0 = ADT_LOGGER_COL_RED;
            break;

            case LEVEL::WARN:
            svCol0 = ADT_LOGGER_COL_YELLOW;
            break;

            case LEVEL::INFO:
            svCol0 = ADT_LOGGER_COL_BLUE;
            break;

            case LEVEL::DEBUG:
            svCol0 = ADT_LOGGER_COL_CYAN;
            break;
        }
        svCol1 = ADT_LOGGER_COL_NORM;
    }

    return print::toSpan(spBuff, "({}{}{}: {}, {}): ", svCol0, eLevel, svCol1, print::stripSourcePath(loc.file_name()), loc.line());
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
