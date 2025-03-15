#include "adt/math.hh"
#include "adt/Arena.hh"
#include "adt/defer.hh"

#include "adt/VecSOA.hh"

using namespace adt;

struct Entity
{
    struct Bind
    {
        math::V3& pos;
        math::V3& scale;

        int& assetI;
    };

    /* */

    math::V3 pos {};
    math::V3 scale {};

    int assetI {};
};

int
main()
{
    Arena arena(SIZE_1K);
    defer( arena.freeAll() );

    VecSOA<Entity, Entity::Bind, &Entity::pos, &Entity::scale, &Entity::assetI> vec(&arena);

    Entity en0 {.pos = {-0.1f, -0.1f, -0.1f}, .scale = {0.1f, 0.1f, 0.1f}, .assetI = -1};
    Entity en1 {.pos = {-1.0f, -1.0f, -1.0f}, .scale = {1.0f, 1.0f, 1.0f}, .assetI = 11};
    Entity en2 {.pos = {-2.0f, -2.0f, -2.0f}, .scale = {2.0f, 2.0f, 2.0f}, .assetI = 22};
    Entity en3 {.pos = {-3.0f, -3.0f, -3.0f}, .scale = {3.0f, 3.0f, 3.0f}, .assetI = 33};

    vec.push(&arena, en0);
    vec.push(&arena, en0);
    vec.push(&arena, en2);
    vec.push(&arena, en3);
    vec.push(&arena, en0);
    vec.push(&arena, en0);
    vec.push(&arena, en0);
    vec.push(&arena, en1);
    vec.push(&arena, en0);
    vec.push(&arena, en0);
    vec.push(&arena, en0);
    vec.push(&arena, en0);

    auto* pos = ((math::V3*)vec.m_pData);
    auto* scale = (math::V3*)(pos + vec.m_cap);
    auto* assetI = (int*)(scale + vec.m_cap);

    print::out("pos: {}\n", *pos);
    print::out("scale: {}\n", *scale);
    print::out("assetI: {}\n", *assetI);

    print::out("\n");

    auto b3 = vec[7];

    auto* pos3 = ((math::V3*)vec.m_pData + 3);
    auto* scale3 = (math::V3*)(pos + vec.m_cap + 3);
    auto* assetI3 = ((int*)(scale + vec.m_cap) + 3);

    print::out("pos: {}\n", *pos3);
    print::out("scale: {}\n", *scale3);
    print::out("assetI: {}\n", *assetI3);

    print::out("\n");

    print::out("pos: {}\n", b3.pos);
    print::out("scale: {}\n", b3.scale);
    print::out("assetI: {}\n", b3.assetI);
}
