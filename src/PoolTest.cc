#include "adt/ReverseIt.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Pool.hh"

using namespace adt;

int
main()
{
    Pool<long, 32> p(INIT);

    [[maybe_unused]] auto h32 = p.push(32L);
    [[maybe_unused]] auto h15 = p.push(15L);

    [[maybe_unused]] auto h1 = p.push(1L);
    [[maybe_unused]] auto h2 = p.push(2L);
    [[maybe_unused]] auto h3 = p.push(3L);
    [[maybe_unused]] auto h4 = p.emplace(4L);

    p.giveBack(h32);
    p.giveBack(h3);

    for (const auto& h : p)
    {
        LOG("h: {}\n", h);
        /*h = 0;*/
    }

    COUT("p: [{}]\n", p);
}
