#include "adt/logs.hh"
#include "adt/Arena.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"
#include "adt/OsAllocator.hh"

using namespace adt;

void
throws()
{
    OsAllocator osAl {};
    auto* ptr = osAl.zalloc(1, SIZE_8G * 1024);
    defer( osAl.free(ptr) );
}

int
main()
{
    try
    {
        throws();
    }
    catch (IException& pEx)
    {
        pEx.logErrorMsg();
    }
}
