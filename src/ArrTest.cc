#include "adt/Arr.hh"
#include "adt/logs.hh"

using namespace adt;

int
main()
{
    Arr<long, 32> arr;

    arr.push(1);
    arr.push(2);
    arr.push(3);
    arr.push(4);
    arr.push(5);

    COUT("arr: {}\n", arr);

    int i = 10;
}
