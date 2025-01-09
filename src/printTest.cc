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

    print::out("'{:10}'", 1);
    print::out("single quote should be before the word 'single'\n");
}
