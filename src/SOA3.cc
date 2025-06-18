#include "adt/StdAllocator.hh"
#include "adt/Vec.hh"
#include "adt/VecSOA.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/math.hh"

using namespace adt;

struct Entity
{
    struct Bind
    {
        math::V3& pos;
        math::V3& scale;
        math::V3& vel;

        math::V3& _pad0;
        math::V3& _pad1;
        math::V3& _pad2;
        math::V3& _pad3;
        math::V3& _pad4;
        math::V3& _pad5;
        math::V3& _pad6;

        int& assetI;
    };

    /* */

    math::V3 pos {};
    math::V3 scale {};
    math::V3 vel {};

    math::V3 _pad0 {};
    math::V3 _pad1 {};
    math::V3 _pad2 {};
    math::V3 _pad3 {};
    math::V3 _pad4 {};
    math::V3 _pad5 {};
    math::V3 _pad6 {};

    int assetI {};
};

int
main()
{
    constexpr isize SIZE = 1000000;

    StdAllocator alloc;

    {
        VecSOA<Entity, Entity::Bind,
            &Entity::pos, &Entity::scale, &Entity::vel,
            &Entity::_pad0,
            &Entity::_pad1,
            &Entity::_pad2,
            &Entity::_pad3,
            &Entity::_pad4,
            &Entity::_pad5,
            &Entity::_pad6,
            &Entity::assetI
        > v0 {&alloc, SIZE};

        defer( v0.destroy(&alloc) );

        for (isize i = 0; i < SIZE; ++i)
        {
            f32 fi = f32(i);
            v0.push(&alloc, {.pos {fi, fi, fi}, .scale {fi, fi, fi}, .vel {fi, fi, fi}, .assetI = int(i)});
        }

        Entity acc {};

        isize t0 = utils::timeNowUS();
        for (const Entity::Bind e : v0)
        {
            acc.pos += e.pos;
            acc.scale += e.pos;
            acc.vel += e.vel;
            acc.assetI += e.assetI;
        }

        isize t1 = utils::timeNowUS() - t0;
        CERR("acc: {}, {}, {}, {}\n", acc.pos, acc.scale, acc.vel, acc.assetI);

        LOG_NOTIFY("SOA time: {:.3} ms\n\n", t1 / 1'000.0);
    }

    {
        VecM<Entity> v0 {SIZE};
        defer( v0.destroy() );

        for (isize i = 0; i < SIZE; ++i)
        {
            f32 fi = f32(i);
            v0.push({.pos {fi, fi, fi}, .scale {fi, fi, fi}, .vel {fi, fi, fi}, .assetI = int(i)});
        }

        Entity acc {};

        isize t0 = utils::timeNowUS();
        for (const auto& e : v0)
        {
            acc.pos += e.pos;
            acc.scale += e.pos;
            acc.vel += e.vel;
            acc.assetI += e.assetI;
        }

        isize t1 = utils::timeNowUS() - t0;
        CERR("acc: {}, {}, {}, {}\n", acc.pos, acc.scale, acc.vel, acc.assetI);

        LOG_NOTIFY("AOS time: {:.3} ms\n", t1 / 1'000.0);
    }
}
