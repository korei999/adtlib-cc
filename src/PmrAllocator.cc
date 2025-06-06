#include "adt/logs.hh"
#include "adt/Vec.hh"
#include "adt/StdAllocator.hh"
#include "adt/defer.hh"

using namespace adt;

int
main()
{
    LOG_NOTIFY("PmrAllocator test...\n");

    VecPmr<int, StdAllocatorNV> v {};
    v.setCap(12);

    defer( v.destroy() );

    v.push(1);
    v.push(2);
    v.push(3);
    v.push(4);
    v.push(5);

    COUT("v: [{}]\n", v);

    LOG_GOOD("PmrAllocator passed.\n");
}
