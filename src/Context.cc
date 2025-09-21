#include "WIP/Context.hh"

#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1 << 12, true};
    defer( logger.destroy() );
    ILogger::setGlobal(&logger);
}
