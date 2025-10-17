#include "adt/Logger.hh"
#include "adt/Arena.hh"
#include "adt/ThreadPool.hh"

using namespace adt;

struct One
{
    ~One() { LogInfo{"~One\n"}; }
};

struct Two
{
    ~Two() { LogInfo{"~Two\n"}; }
};

union What
{
    One one;
    Two two;

    ~What()
    {
        one.~One();
        LogInfo{"~What\n"};
    }
};

static void
test()
{
    What w {One{}};
}

int
main()
{
    ThreadPool ztp {Arena{}, SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer( ztp.destroy() );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    try
    {
        test();
    }
    catch (const std::exception& ex)
    {
        LogError{"{}\n", ex.what()};
    }
}
