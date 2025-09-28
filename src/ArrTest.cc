#include "adt/Array.hh"
#include "adt/Gpa.hh"
#include "adt/sort.hh"
#include "adt/Logger.hh"

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

    print::out("arr: {}\n", arr);

    [[maybe_unused]] int i = 10;

    {
        Array<int, 16> a {};

        sort::push<sort::ORDER::INC>(&a, 5);
        sort::push<sort::ORDER::INC>(&a, 4);
        sort::push<sort::ORDER::INC>(&a, 10);
        sort::push<sort::ORDER::INC>(&a, 2);
        sort::push<sort::ORDER::INC>(&a, -1);
        sort::push<sort::ORDER::INC>(&a, 50);
        sort::push<sort::ORDER::INC>(&a, -240);

        ADT_ASSERT_ALWAYS(sort::sorted(a, sort::ORDER::INC), "");

        print::err("sorted inc: {}\n", a);
    }

    {
        constexpr Array<int, 4> a {0, 1, 2, 3};
        ADT_ASSERT_ALWAYS(a[0] == 0, "");
        ADT_ASSERT_ALWAYS(a[1] == 1, "");
        ADT_ASSERT_ALWAYS(a[2] == 2, "");
        ADT_ASSERT_ALWAYS(a[3] == 3, "");
    }
}
