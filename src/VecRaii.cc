#include "adt/logs.hh"

#include "rdt/types.hh"
#include "rdt/Vec.hh"

using namespace rdt;

struct What
{
    int i {};

    What(int _i) : i {_i} {}

    What(const What& other) : i {other.i} { COUT("copied: {}\n", i); }

    What(What&& other) : i {std::exchange(other.i, 0)} { COUT("moved: {}\n", i); }

    ~What() { if (i != 0) COUT("what #{} dies\n", i); }
};

int
main()
{
    Vec<What> v {5};

    v.emplace(1);
    v.emplace(2);
    v.emplace(3);
    v.emplace(4);
    v.emplace(5);
    v.emplace(6);

    // for (ssize i = 0; i < v.size(); ++i)
    // {
    //     COUT("#{}: {}\n", i, v[i].i);
    // }
}
