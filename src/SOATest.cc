#include "adt/logs.hh"
#include "adt/math.hh"

using namespace adt;

/* Example struct. */
struct Entity
{
    math::V3 pos {};
    math::Qt rot {};
    math::V3 scale {};
    int id {};

    Entity() = delete;
    Entity(math::V3 _pos, math::Qt _rot, math::V3 _scale, int _id)
        : pos(_pos), rot(_rot), scale(_scale), id(_id) {}
};

/* Bind type: a struct of references that will be returned by bind. */
struct EntityBind
{
    math::V3& pos;
    math::Qt& rot;
    math::V3& scale;
    int& id;
};

namespace adt::print
{

inline isize
formatToContext(Context ctx, FormatArgs, const EntityBind& x)
{
    ctx.fmt = "[{}], [{}], [{}], {}";
    ctx.fmtIdx = 0;
    return printArgs(ctx, x.pos, x.rot, x.scale, x.id);
}

} /* namespace adt::print */

/* Helper base class to store one array corresponding to a specific bindMember.
 * T: the full type (e.g. Entity)
 * CAP: capacity
 * Member: a pointer-to-bindMember (e.g. &Entity::pos) */
template<typename T, isize CAP, auto MEMBER>
struct SOAArrayHolder
{
    /* Deduce the type of the bindMember. */
    using MemberType = std::remove_reference_t<decltype(std::declval<T>().*MEMBER)>;

    /* */

    MemberType m_arrays[CAP] {};
};

/* The main SOA container, which inherits from a SOA_ArrayHolder for each bindMember. */
template<typename STRUCT, typename BIND, isize CAP, auto ...MEMBERS>
struct SOA : public SOAArrayHolder<STRUCT, CAP, MEMBERS>...
{
    /* Set the element at index 'idx' from a full object. */
    void
    set(isize idx, const STRUCT& value)
    {
        /* Expand the pack: for each Member, assign value.*Member to the corresponding array element. */
        ((static_cast<SOAArrayHolder<STRUCT, CAP, MEMBERS>&>(*this).m_arrays[idx] = value.*MEMBERS), ...);
    }

    /* Helper: get a reference for a specific member. */
    template<auto MEMBER>
    decltype(auto)
    bindMember(isize idx)
    {
        return static_cast<SOAArrayHolder<STRUCT, CAP, MEMBER>&>(*this).m_arrays[idx];
    }

    template<auto MEMBER>
    decltype(auto)
    bindMember(isize idx) const
    {
        return static_cast<const SOAArrayHolder<STRUCT, CAP, MEMBER>&>(*this).m_arrays[idx];
    }

    /* bind returns a Bind struct (e.g. EntityBind) composed of references to the underlying arrays.
     * Bind must be aggregate initializable from the members in the same order as Members. */
    BIND
    bind(isize idx)
    {
        return BIND {bindMember<MEMBERS>(idx)...};
    }

    BIND operator[](isize idx) { return bind(idx); }
    const BIND operator[](isize idx) const { return bind(idx); }
};

static void what();

int
main()
{
    // Create an SOA container for Entity.
    // The order of pointer-to-members determines the order of the stored arrays.
    SOA<Entity, EntityBind, 10, &Entity::pos, &Entity::rot, &Entity::scale, &Entity::id> pool;
    
    // Set element 0.
    {
        Entity e {math::V3From(1.1f, 2.2f, 3.3f), math::Qt{{{0.f, 0.f, 0.f, 1.f}}}, math::V3From(0.0f, 0.1f, 0.2f), 42};
        pool.set(0, e);
    }

    {
        auto first = pool.bind(0);
        LOG("first: {}\n", first);
        first.id = 999;
    }

    {
        auto first = pool.bind(0);
        LOG("first: {}\n", first);
        auto& id = pool.bindMember<&Entity::id>(0);
        id = -1;
    }

    auto first = pool.bind(0);
    LOG("first: {}\n", first);

    pool[0].id = 666;

    LOG("first: {}\n", first);

    what();
}

struct WhatBind
{
    int& a, & b, & c, & d;
};

struct What
{
    int a, b, c, d;
};

template<typename STRUCT, auto MEMBER>
struct WhatHolder
{
    using MemberType = std::remove_reference_t<decltype(std::declval<STRUCT>().*MEMBER)>;

    /* */

    MemberType m_holder {};
};

template<typename STRUCT, typename BIND, auto ...MEMBERS>
struct WhatIsThis : public WhatHolder<STRUCT, MEMBERS>...
{
    void
    set(int _i)
    {
        ((static_cast<WhatHolder<STRUCT, MEMBERS>&>(*this).m_holder = _i), ...);
    }

    template<auto MEMBER>
    auto&
    bindMember()
    {
        return static_cast<WhatHolder<STRUCT, MEMBER>&>(*this).m_holder;
    }

    BIND
    bind()
    {
        return BIND {bindMember<MEMBERS>()...};
    }
};

static void
what()
{
    WhatIsThis<What, WhatBind, &What::a, &What::b, &What::c, &What::d> what {};
    what.set(10);
    what.set(11);

    auto a = what.bindMember<&What::a>();
    LOG("a: {}\n", a);

    WhatBind bind = what.bind();
    bind.a = 5;

    a = what.bindMember<&What::a>();
    auto& b = what.bindMember<&What::b>();
    LOG("a: {}, b: {}\n", a, b);
    b = -1;
    LOG("a: {}, b: {}\n", a, b);
}
