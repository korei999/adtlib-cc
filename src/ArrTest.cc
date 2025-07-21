#include "adt/Array.hh"
#include "adt/StdAllocator.hh"
#include "adt/logs.hh"

#include "Types.hh"

using namespace adt;

int
main()
{
    Array<long, 32> arr;

    arr.push(1);
    arr.push(2);
    arr.push(3);
    arr.push(4);
    arr.push(5);

    COUT("arr: {}\n", arr);

    [[maybe_unused]] int i = 10;

    {
        Array<int, 16> a {};

        a.pushSorted(sort::ORDER::INC, 5);
        a.pushSorted(sort::ORDER::INC, 4);
        a.pushSorted(sort::ORDER::INC, 10);
        a.pushSorted(sort::ORDER::INC, 2);
        a.pushSorted(sort::ORDER::INC, -1);
        a.pushSorted(sort::ORDER::INC, 50);
        a.pushSorted(sort::ORDER::INC, -240);

        LOG("sorted inc: {}\n", a);
    }

    {
        constexpr Array<int, 4> a {0, 1, 2, 3};
        ADT_ASSERT_ALWAYS(a[0] == 0, "");
        ADT_ASSERT_ALWAYS(a[1] == 1, "");
        ADT_ASSERT_ALWAYS(a[2] == 2, "");
        ADT_ASSERT_ALWAYS(a[3] == 3, "");
    }
}
