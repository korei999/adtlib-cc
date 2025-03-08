import adt.Malloc;
import adt.print;
import adt.Vec;

#include "adt/defer.hh"

#include <cstdio>

int
main()
{
    adt::VecManaged<int> vec(adt::MallocGet());
    defer( vec.destroy() );
    vec.push(1);
    vec.push(-1);
    vec.push(2);
    vec.push(-2);

    for (int e : vec)
        adt::print::out("e: {}\n", e);
}
