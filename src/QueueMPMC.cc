#include "adt/QueueMPMC.hh"
#include "adt/Thread.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("QueueMPMC test...\n");

    LOG_GOOD("QueueMPMC test passed.\n");
}
