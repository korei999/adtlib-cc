#include "adt/Arena.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/String.hh"

using namespace adt;

int
main()
{
    int nSpaces = 2;

    print::out("{:{}}", nSpaces, "");
    print::out("there must be {} spaces before this string\n", nSpaces);
}
