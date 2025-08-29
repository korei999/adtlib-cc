#include "adt/ArenaList.hh"
#include "adt/ReverseIt.hh"
#include "adt/VecSOA.hh"
#include "adt/defer.hh"
#include "adt/math.hh"

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
    ArenaList arena(SIZE_1K);
    defer( arena.freeAll() );

    VecSOA<Entity, Entity::Bind, &Entity::pos, &Entity::scale, &Entity::assetI> vec(&arena);

    Entity en0 {.pos = {-0.1f, -0.1f, -0.1f}, .scale = {0.1f, 0.1f, 0.1f}, .assetI = -1};
    Entity en1 {.pos = {-1.0f, -1.0f, -1.0f}, .scale = {1.0f, 1.0f, 1.0f}, .assetI = 11};
    Entity en2 {.pos = {-2.0f, -2.0f, -2.0f}, .scale = {2.0f, 2.0f, 2.0f}, .assetI = 22};
    Entity en3 {.pos = {-3.0f, -3.0f, -3.0f}, .scale = {3.0f, 3.0f, 3.0f}, .assetI = 33};

    vec.push(&arena, en0);
    vec.push(&arena, en1);
    vec.push(&arena, en2);
    vec.push(&arena, en3);
    vec.push(&arena, en0);
    vec.push(&arena, en1);
    vec.push(&arena, en2);
    vec.push(&arena, en3);
    vec.push(&arena, en0);
    vec.push(&arena, en1);
    vec.push(&arena, en2);
    vec.push(&arena, en3);

    int i = 0;
    for (auto bind : vec)
    {
        print::out("i: {}\n", i);
        print::out("pos: {}\n", bind.pos);
        print::out("scale: {}\n", bind.scale);
        print::out("assetI: {}\n\n", bind.assetI);

        ++i;
    }

    i = 0;
    for (auto bind : ReverseIt(vec))
    {
        print::out("i: {}\n", i);
        print::out("pos: {}\n", bind.pos);
        print::out("scale: {}\n", bind.scale);
        print::out("assetI: {}\n\n", bind.assetI);

        ++i;
    }
}
