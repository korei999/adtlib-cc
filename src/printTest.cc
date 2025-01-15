#include "adt/Arena.hh" /* IWYU pragma: keep */
#include "adt/defer.hh" /* IWYU pragma: keep */
#include "adt/logs.hh" /* IWYU pragma: keep */
#include "adt/Vec.hh" /* IWYU pragma: keep */
#include "adt/List.hh" /* IWYU pragma: keep */
#include "adt/String.hh"

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

    print::out("precision: {}, float: {}, {:.10}\n", 10, M_PI, M_PI);
}
