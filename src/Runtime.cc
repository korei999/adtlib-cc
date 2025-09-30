#include "WIP/Runtime.hh"

#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {2, ILogger::LEVEL::DEBUG, 1 << 12, true};
    defer( logger.destroy() );
    ILogger::setGlobal(&logger);
}
