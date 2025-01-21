#include "adt/Arena.hh" /* IWYU pragma: keep */
#include "adt/Arr.hh"
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/logs.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh" /* IWYU pragma: keep */
#include "adt/List.hh" /* IWYU pragma: keep */
#include "adt/String.hh"
#include "adt/math.hh"

using namespace adt;

int
main()
{
    int nSpaces = 2;

    print::out("{:{}}", nSpaces, "");
    print::out("there must be {} spaces before this string\n", nSpaces);

    print::out("'{:>10}'\n", "10");

    print::out("'{:{}}'", 10, 1);
    print::out("there must be single quote before the word 'there'\n");

    print::out("dec: {}, hex: {:#x}, bin: {:#b}\n", 13, 13, 13);

    print::out("precision: {}, float: {}, {:.10}\n", 10, math::PI64, math::PI64);

    print::out("{}\n", 10);


    Arr<f64, 32> arr {1.1, 2.2, 3.3, 4.4, 5.5};
    print::out("arr: [{:.5}]\n", arr);

    Span<f64> spArr(arr.data(), arr.getSize());
    print::out("spArr: [{:.4}]\n", spArr);
}
