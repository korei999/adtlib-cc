#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Pool.hh"

using namespace adt;

int
main()
{
    Pool<int, 16> p {INIT};

    [[maybe_unused]] auto h32 = p.insert(32);
    [[maybe_unused]] auto h15 = p.insert(15);

    [[maybe_unused]] auto h1 = p.insert(1);
    [[maybe_unused]] auto h2 = p.insert(2);
    [[maybe_unused]] auto h3 = p.insert(3);
    [[maybe_unused]] auto h4 = p.emplace(4);

    p.remove(h32);
    p.remove(h3);

    for (const auto& h : p)
    {
        LOG("#{}: h: {}\n", p.idx(&h), h);
    }
}
