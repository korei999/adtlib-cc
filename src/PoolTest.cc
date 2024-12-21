#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/Pool.hh"

using namespace adt;

int
main()
{
    Pool<long, 32> p(INIT_FLAG::INIT);
    defer( p.destroy() );

    [[maybe_unused]] auto h32 = p.getHandle(32L);
    [[maybe_unused]] auto h15 = p.getHandle(15L);

    [[maybe_unused]] auto h1 = p.getHandle(1L);
    [[maybe_unused]] auto h2 = p.getHandle(2L);
    [[maybe_unused]] auto h3 = p.getHandle(3L);

    p.giveBack(h32);
    /*p.giveBack(h15);*/

    for (const auto& h : p)
        LOG("h: {}\n", h);

}
