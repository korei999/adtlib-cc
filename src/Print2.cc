#include "adt/Arena.hh"
#include "adt/Array.hh"
#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

template<isize N>
struct TypeErasedArray
{
    using PfnDispatch = isize (*)(print::Context* pCtx, print::FormatArgs fmtArgs, void* p);

    struct Formatter
    {
        PfnDispatch pfn;
        void* ptr;
    };

    /* */

    Array<Formatter, N> m_aData {};

    /* */

    template<typename ...ARGS>
    constexpr TypeErasedArray(const ARGS&... args)
        : m_aData{makeFormatterHelper<ARGS>(args)...}
    {
        for (auto& e : m_aData)
            e.pfn({}, {}, e.ptr);
    }

    template<typename T>
    static constexpr Formatter
    makeFormatterHelper(const T& v)
    {
        constexpr PfnDispatch fn = [](print::Context* pCtx, print::FormatArgs fmtArgs, void* pArg) {
            auto* p = static_cast<T*>(pArg);
            return print::out("what: {}\n", *p);
        };

        return Formatter{ fn, const_cast<void*>(static_cast<const void*>(&v)) };
    }
};

template<typename ...ARGS>
static constexpr void
TypeErasedArrayPrint(const ARGS&... args)
{
    TypeErasedArray<sizeof...(ARGS)>{args...};
}

static void
go()
{
    TypeErasedArrayPrint("hello", 2, 3, std::initializer_list{7, 7, 7, 7});
}

int
main()
{

    ThreadPool ztp {Arena{}, SIZE_1G*16};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"Print2 test...\n"};
    try
    {
        go();
    }
    catch (std::exception& ex)
    {
        LogError{"{}\n", ex.what()};
    }
    LogInfo{"Print2 test done\n"};
}
