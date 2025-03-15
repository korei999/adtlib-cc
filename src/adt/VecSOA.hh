#pragma once

#include "IAllocator.hh"
#include "utils.hh"

namespace adt
{

/* TODO: i think we can generate this just from one Entity without the mirror. */

/* This template allows to create arbitrary vectors with SOA memory layout.
 * Creation example:
 *
 * struct Entity
 * {
 *     // reference mirror
 *     struct Bind
 *     {
 *         math::V3& pos;
 *         math::V3& vel;
 *         int& index;
 *     }
 *     
 *     math::V3 pos {};
 *     math::V3 vel {};
 *     int index {};
 * };
 * 
 * Must preserve the order.
 * VecSOA<Entity, Entity::Bind, &Entity::pos, &Entity::vel, &Entity::index> vec(StdAllocator::inst());
 * 
 * Very ugly, gets the job done. */

template<typename STRUCT, typename BIND, auto ...MEMBERS>
struct VecSOA
{
    u8* m_pData {};
    ssize m_size {};
    ssize m_cap {};

    /* */

    VecSOA() = default;
    VecSOA(IAllocator* pAlloc, ssize prealloc = SIZE_MIN);

    /* */

    BIND operator[](const ssize i) { return bind(i); }
    const BIND operator[](const ssize i) const { return bind(i); }

    BIND bind(const ssize i) const;
    ssize push(IAllocator* pAlloc, const STRUCT& x);

protected:
    void set(ssize i, const STRUCT& x);
    void growIfNeeded(IAllocator* pAlloc);
    void grow(IAllocator* p, ssize newCapacity);
};

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline
VecSOA<STRUCT, BIND, MEMBERS...>::VecSOA(IAllocator* pAlloc, ssize prealloc)
    : m_pData(pAlloc->mallocV<u8>(prealloc * sizeof(STRUCT))), m_cap(prealloc) {}

template<typename STRUCT, typename BIND>
inline void VecSOASetHelper(u8*, const ssize, const ssize) { /* empty case */ }

template<typename STRUCT, typename BIND, typename HEAD, typename ...TAIL>
inline void
VecSOASetHelper(u8* pData, const ssize cap, const ssize i, HEAD&& head, TAIL&&... tail)
{
    using HeadType = std::remove_reference_t<HEAD>;

    auto* pPlacement = reinterpret_cast<HeadType*>(pData) + i;
    new( (void*)(pPlacement) ) HeadType(head);

    VecSOASetHelper<STRUCT, BIND>(
        (u8*)((reinterpret_cast<HeadType*>(pData)) + cap), cap, i, std::forward<TAIL>(tail)...
    );
}

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline void
VecSOA<STRUCT, BIND, MEMBERS...>::set(ssize i, const STRUCT& x)
{
    ADT_ASSERT(i >= 0 && i < m_size, "out of range: i: %lld, size: %lld\n", i, m_size);
    VecSOASetHelper<STRUCT, BIND>(m_pData, m_cap, i, x.*MEMBERS...);
}

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline ssize
VecSOA<STRUCT, BIND, MEMBERS...>::push(IAllocator* pAlloc, const STRUCT& x)
{
    growIfNeeded(pAlloc);
    ++m_size;
    set(m_size - 1, x);
    return m_size - 1;
}

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline void
VecSOA<STRUCT, BIND, MEMBERS...>::growIfNeeded(IAllocator* pAlloc)
{
    if (m_size < m_cap) return;

    ssize newCap = utils::max(decltype(m_cap)(SIZE_MIN), m_cap*2);
    ADT_ASSERT(newCap > m_cap, "can't grow (capacity overflow), newCap: %lld, m_capacity: %lld", newCap, m_cap);
    grow(pAlloc, newCap);
}

template<typename STRUCT, typename BIND>
inline void VecSOAReallocHelper(const u8* const, u8*, const ssize, const ssize) { /* empty case */ }

template<typename STRUCT, typename BIND, typename HEAD, typename ...TAIL>
inline void
VecSOAReallocHelper(const u8* const pOld, u8* pData, const ssize oldCap, const ssize newCap)
{
    using HeadType = std::remove_reference_t<HEAD>;

    memcpy(pData, pOld, oldCap * sizeof(HeadType));

    u8* pNextOld = (u8*)(reinterpret_cast<const HeadType*>(pOld) + oldCap);
    u8* pNextData = (u8*)(reinterpret_cast<const HeadType*>(pData) + newCap);

    VecSOAReallocHelper<STRUCT, BIND, TAIL...>(
        pNextOld, pNextData, oldCap, newCap
    );
}

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline void
VecSOA<STRUCT, BIND, MEMBERS...>::grow(IAllocator* p, ssize newCapacity)
{
    u8* pNewData = p->mallocV<u8>(newCapacity * sizeof(STRUCT));

    VecSOAReallocHelper<STRUCT, BIND, decltype(std::declval<STRUCT>().*MEMBERS)...>(
        m_pData, pNewData, m_cap, newCapacity
    );

    m_cap = newCapacity;
    p->free(m_pData);
    m_pData = pNewData;
}

template<typename STRUCT, typename BIND, auto ...MEMBERS>
inline BIND
VecSOA<STRUCT, BIND, MEMBERS...>::bind(const ssize i) const
{
    ADT_ASSERT(i >= 0 && i < m_size, "out of range: i: %lld, size: %lld\n", i, m_size);

    u8* p = m_pData;
    ssize off = 0;

    return {
        ([&]() -> auto& {
            using FieldType = std::remove_reference_t<decltype(std::declval<STRUCT>().*MEMBERS)>;

            auto& ref = reinterpret_cast<FieldType*>(p + off)[i];

            off += m_cap * sizeof(FieldType);
            return ref;
        }())...
    };
}

} /* namespace adt */
