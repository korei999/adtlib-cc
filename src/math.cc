#include "adt/Logger.hh"
#include "adt/math.hh"

using namespace adt;

int
main()
{
    math::V2 v0 {1, 2};
    math::V2 v1 = 3.3f * v0;
    math::M3 m3_0 = math::M3Iden();
    math::V3 v3_0 {};

    m3_0 * v3_0;
}
