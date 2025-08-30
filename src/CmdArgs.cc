#include "CmdArgs.hh"

#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 2048, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"CmdArgs test...\n"};

    LogInfo{"CmdArgs test passed\n"};
}
