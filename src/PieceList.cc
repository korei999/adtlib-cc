#include "PieceList.hh"

#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 1024, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"PieceList test...\n"};



    LogInfo{"PieceList test passed\n"};
}
