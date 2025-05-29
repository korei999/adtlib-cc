#pragma once

#include "hash.hh"
#include "Vec.hh"

namespace adt
{

constexpr f32 SET_DEFAULT_LOAD_FACTOR = 0.5f;
constexpr f32 SET_DEFAULT_LOAD_FACTOR_INV = 1.0f / SET_DEFAULT_LOAD_FACTOR;

/* Like adt::Map but key is also a value. */
template<typename T, usize (*FN_HASH)(const T&) = hash::func<T>>
struct Set
{
    static constexpr f32 DEFAULT_LOAD_FACTOR = 0.5f;
    static constexpr f32 DEFAULT_LOAD_FACTOR_INV = 1.0f / DEFAULT_LOAD_FACTOR;

    struct Bucket
    {
        T val {};
        bool bOccupied = false;
        bool bDeleted = false;
    };

    /* custom return type for insert/search operations */
    struct Result
    {
        enum class STATUS : u8 { NOT_FOUND, FOUND, INSERTED };

        /* */

        Bucket m_val {};
        usize m_hash {};
        STATUS m_eStatus {};

        explicit constexpr operator bool() const
        {
            return m_eStatus != STATUS::NOT_FOUND;
        }

        constexpr const T&
        value() const
        {
            return m_val.val;
        }

        constexpr const T
        valueOr(T&& _or) const
        {
            if (operator bool())
                return value();
            else return std::forward<T>(_or);
        }
    };

    /* */

    Vec<Bucket> m_vBuckets {};
    isize m_nOccupied {};
    f32 m_maxLoadFactor {};

    /* */

    Set() = default;
    Set(IAllocator* pAllocator, isize prealloc = SIZE_MIN);

    /* */
};

template<typename T, usize (*FN_HASH)(const T&)>
Set<T, FN_HASH>::Set(IAllocator* pAllocator, isize prealloc)
    : m_vBuckets {pAllocator, isize(prealloc * DEFAULT_LOAD_FACTOR_INV)},
      m_maxLoadFactor {DEFAULT_LOAD_FACTOR}
{
    m_vBuckets.setSize(pAllocator, prealloc * DEFAULT_LOAD_FACTOR_INV);
    m_vBuckets.zeroOut();
}

} /* namespace adt */
