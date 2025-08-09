#include "adt/SOA.hh"
#include "adt/StdAllocator.hh"
#include "adt/Vec.hh"
#include "adt/VecSOA.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"
#include "adt/math.hh"
#include "adt/time.hh"

using namespace adt;

#define ENTITY_PP_BIND_I(TYPE, NAME) , &Entity::NAME
#define ENTITY_PP_BIND(TUPLE) ENTITY_PP_BIND_I TUPLE
#define ENTITY_FIELDS \
    (math::V3, pos),\
    (math::V3, scale),\
    (math::V3, vel),\
    (math::V3, _pad0),\
    (math::V3, _pad1),\
    (math::V3, _pad2),\
    (math::V3, _pad3),\
    (math::V3, _pad4),\
    (math::V3, _pad5),\
    (math::V3, _pad6),\
    (int, assetI),\
    (math::V4, _pad7),\
    (math::V4, _pad8),\
    (math::V4, _pad9),\
    (math::V4, _pad10),\
    (math::V4, _pad11),\
    (math::V4, _pad12),\
    (math::V4, _pad13),\
    (math::V4, _pad14)
ADT_SOA_GEN_STRUCT_ZERO(Entity, Bind, ENTITY_FIELDS);
#define ENTITY_TEMPLATE_ARGS Entity, Entity::Bind ADT_PP_FOR_EACH(ENTITY_PP_BIND, ENTITY_FIELDS)

int
main()
{
    constexpr isize SIZE = 5000000;

    StdAllocator alloc;

    {
        VecSOA<ENTITY_TEMPLATE_ARGS> v0 {&alloc, SIZE};
        defer( v0.destroy(&alloc) );

        for (isize i = 0; i < SIZE; ++i)
        {
            f32 fi = f32(i);
            v0.push(&alloc, {
                .pos {fi, fi, fi}, .scale {fi, fi, fi}, .vel {fi, fi, fi}, .assetI = int(i)});
        }

        Entity acc {};

        isize t0 = time::nowUS();
        for (Entity::Bind en : v0)
        {
            acc.pos += en.pos;
            acc.scale += en.scale;
            acc.vel += en.vel;
            acc.assetI += en.assetI;
        }

        isize t1 = time::nowUS() - t0;
        CERR("acc: {:.3}, {:.3}, {:.3}, {:.3}\n", acc.pos, acc.scale, acc.vel, acc.assetI);

        LOG_NOTIFY("SOA time: {:.3} ms\n\n", t1 / 1'000.0);
    }

    {
        VecSOA<Entity, Entity::Bind
            ADT_PP_FOR_EACH(ENTITY_PP_BIND, ENTITY_FIELDS)
        > v0 {&alloc, SIZE};

        defer( v0.destroy(&alloc) );

        for (isize i = 0; i < SIZE; ++i)
        {
            f32 fi = f32(i);
            v0.push(&alloc, {.pos {fi, fi, fi}, .scale {fi, fi, fi}, .vel {fi, fi, fi}, .assetI = int(i)});
        }

        Entity acc {};

        auto* pPos = &v0[0].pos;
        auto* pScale = &v0[0].scale;
        auto* pVel = &v0[0].vel;
        auto* pAsset = &v0[0].assetI;

        isize t0 = time::nowUS();
        for (isize i = 0; i < v0.size(); ++i)
        {
            acc.pos += pPos[i];
            acc.scale += pScale[i];
            acc.vel += pVel[i];
            acc.assetI += pAsset[i];
        }

        isize t1 = time::nowUS() - t0;
        CERR("acc: {:.3}, {:.3}, {:.3}, {:.3}\n", acc.pos, acc.scale, acc.vel, acc.assetI);

        LOG_NOTIFY("SOA(offsets) time: {:.3} ms\n\n", t1 / 1'000.0);
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

        isize t0 = time::nowUS();
        for (isize i = 0; i < v0.size(); ++i)
        {
            acc.pos += v0[i].pos;
            acc.scale += v0[i].scale;
            acc.vel += v0[i].vel;
            acc.assetI += v0[i].assetI;
        }

        isize t1 = time::nowUS() - t0;
        CERR("acc: {:.3}, {:.3}, {:.3}, {:.3}\n", acc.pos, acc.scale, acc.vel, acc.assetI);

        LOG_NOTIFY("AOS time: {:.3} ms\n\n", t1 / 1'000.0);
    }
}
