#pragma once

#include "IAllocator.hh"
#include "utils.hh"

namespace adt
{

/* TODO: overflow checks are pretty naive */

/* Dynamic array (aka Vector) */
template<typename T>
struct VecBase
{
    T* m_pData;
    isize m_size;
    isize m_cap;

    /* */

    VecBase() noexcept : m_pData{}, m_size{}, m_cap{} {}
    VecBase(UninitFlag) noexcept {}

    VecBase(IAllocator* p, isize prealloc = SIZE_MIN)
        : m_pData(p->zallocV<T>(prealloc)),
          m_size(0),
          m_cap(prealloc) {}

    VecBase(IAllocator* p, isize preallocSize, const T& fillWith);

    /* */

    explicit operator Span<T>() { return {m_pData, m_size}; }
    explicit operator const Span<const T>() const { return {m_pData, m_size}; }

    /* */

#define ADT_RANGE_CHECK ADT_ASSERT(i >= 0 && i < m_size, "i: {}, m_size: {}", i, m_size);

    [[nodiscard]] T& operator[](isize i)             noexcept { ADT_RANGE_CHECK; return m_pData[i]; }
    [[nodiscard]] const T& operator[](isize i) const noexcept { ADT_RANGE_CHECK; return m_pData[i]; }

#undef ADT_RANGE_CHECK

    [[nodiscard]] bool empty() const noexcept { return m_size <= 0; }

    isize fakePush(IAllocator* p);

    isize push(IAllocator* p, const T& data);
    isize push(IAllocator* p, T&& data);

    void pushAt(IAllocator* p, const isize atI, const T& data);
    void pushAt(IAllocator* p, const isize atI, T&& data);

    isize pushSpan(IAllocator* p, const Span<const T> sp);

    void pushSpanAt(IAllocator* p, const isize atI, const Span<const T> sp);

    template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
    isize emplace(IAllocator* p, ARGS&&... args);

    template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
    void emplaceAt(IAllocator* p, const isize atI, ARGS&&... args);

    [[nodiscard]] T& last() noexcept;

    [[nodiscard]] const T& last() const noexcept;

    [[nodiscard]] T& first() noexcept;

    [[nodiscard]] const T& first() const noexcept;

    T pop() noexcept;

    void setSize(IAllocator* p, isize size);

    void setCap(IAllocator* p, isize cap);

    void swapWithLast(isize i) noexcept;

    void popAsLast(isize i) noexcept;

    void removeAndShift(isize i) noexcept;

    [[nodiscard]] isize idx(const T* const x) const noexcept;

    [[nodiscard]] isize lastI() const noexcept;

    void destroy(IAllocator* p) noexcept;

    [[nodiscard]] VecBase<T> release() noexcept;

    [[nodiscard]] isize size() const noexcept;

    [[nodiscard]] isize cap() const noexcept;

    [[nodiscard]] T* data() noexcept;

    [[nodiscard]] const T* data() const noexcept;

    void zeroOut() noexcept; /* set size to zero and memset */

    [[nodiscard]] VecBase<T> clone(IAllocator* pAlloc) const;

    [[nodiscard]] bool search(const T& x) const;

    /* */

private:
    void grow(IAllocator* p, isize newCapacity);

    void growIfNeeded(IAllocator* p);
    void growOnSpanPush(IAllocator* p, const isize spanSize);

public:

    /* */

    T* begin() noexcept { return {&m_pData[0]}; }
    T* end() noexcept { return {&m_pData[m_size]}; }
    T* rbegin() noexcept { return {&m_pData[m_size - 1]}; }
    T* rend() noexcept { return {m_pData - 1}; }

    const T* begin() const noexcept { return {&m_pData[0]}; }
    const T* end() const noexcept { return {&m_pData[m_size]}; }
    const T* rbegin() const noexcept { return {&m_pData[m_size - 1]}; }
    const T* rend() const noexcept { return {m_pData - 1}; }
};

template<typename T>
inline
VecBase<T>::VecBase(IAllocator* p, isize prealloc, const T& defaultVal)
    : VecBase(p, prealloc)
{
    setSize(p, prealloc);
    for (auto& e : (*this)) new(&e) T {defaultVal};
}

template<typename T>
inline isize
VecBase<T>::fakePush(IAllocator* p)
{
    growIfNeeded(p);
    return ++m_size - 1;
}

template<typename T>
inline isize
VecBase<T>::push(IAllocator* p, const T& data)
{
    growIfNeeded(p);
    new(m_pData + m_size++) T(data);
    return m_size - 1;
}

template<typename T>
inline isize
VecBase<T>::push(IAllocator* p, T&& data)
{
    growIfNeeded(p);
    new(m_pData + m_size++) T {std::move(data)};
    return m_size - 1;
}

template<typename T>
inline void
VecBase<T>::pushAt(IAllocator* p, const isize atI, const T& data)
{
    emplaceAt(p, atI, data);
}

template<typename T>
inline void
VecBase<T>::pushAt(IAllocator* p, const isize atI, T&& data)
{
    emplaceAt(p, atI, std::move(data));
}

template<typename T>
inline isize
VecBase<T>::pushSpan(IAllocator* p, const Span<const T> sp)
{
    growOnSpanPush(p, sp.size());
    utils::memCopy(m_pData + m_size, sp.data(), sp.size());

    const isize ret = m_size;
    m_size += sp.size();

    return ret;
}

template<typename T>
inline void
VecBase<T>::pushSpanAt(IAllocator* p, const isize atI, const Span<const T> sp)
{
    growOnSpanPush(p, sp.size());
    m_size += sp.size();
    ADT_ASSERT(atI >= 0 && atI < size(), "atI: {}, size: {}", atI, size());

    utils::memMove(m_pData + atI + sp.size(), m_pData + atI, size() - sp.size() - atI);
    utils::memCopy(m_pData + atI, sp.data(), sp.size());
}

template<typename T>
template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
inline isize
VecBase<T>::emplace(IAllocator* p, ARGS&&... args)
{
    growIfNeeded(p);
    new(m_pData + m_size++) T(std::forward<ARGS>(args)...);
    return m_size - 1;
}

template<typename T>
template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
inline void
VecBase<T>::emplaceAt(IAllocator* p, const isize atI, ARGS&&... args)
{
    growIfNeeded(p);
    ADT_ASSERT(atI >= 0 && atI < size() + 1, "atI: {}, size + 1: {}", atI, size() + 1);

    utils::memMove<T>(m_pData + atI + 1, m_pData + atI, size() - atI);
    new(&operator[](atI)) T {std::forward<ARGS>(args)...};

    ++m_size;
}

template<typename T>
inline T&
VecBase<T>::last() noexcept
{
    return operator[](m_size - 1);
}

template<typename T>
inline const T&
VecBase<T>::last() const noexcept
{
    return operator[](m_size - 1);
}

template<typename T>
inline T&
VecBase<T>::first() noexcept
{
    return operator[](0);
}

template<typename T>
inline const T&
VecBase<T>::first() const noexcept
{
    return operator[](0);
}

template<typename T>
inline T
VecBase<T>::pop() noexcept
{
    ADT_ASSERT(m_size > 0, "empty");

    T r = std::move(m_pData[--m_size]);
    if constexpr (!std::is_trivially_destructible_v<T>)
        m_pData[m_size].~T();

    return r;
}

template<typename T>
inline void
VecBase<T>::setSize(IAllocator* p, isize size)
{
    if (m_cap < size) grow(p, size);

    m_size = size;
}

template<typename T>
inline void
VecBase<T>::setCap(IAllocator* p, isize cap)
{
    if (cap == 0)
    {
        destroy(p);
        return;
    }

    m_pData = p->relocate<T>(m_pData, m_cap, cap);
    m_cap = cap;

    if (m_size > cap) m_size = cap;
}

template<typename T>
inline void
VecBase<T>::swapWithLast(isize i) noexcept
{
    ADT_ASSERT(m_size > 0, "empty");
    utils::swap(&operator[](i), &operator[](m_size - 1));
}

template<typename T>
inline void
VecBase<T>::popAsLast(isize i) noexcept
{
    ADT_ASSERT(m_size > 0, "empty");

    operator[](i) = std::move(last());

    if constexpr (!std::is_trivially_destructible_v<T>)
        operator[](lastI()).~T();

    --m_size;
}

template<typename T>
inline void
VecBase<T>::removeAndShift(isize i) noexcept
{
    ADT_ASSERT(m_size > 0, "empty");

    if (i != lastI())
        utils::memMove(&operator[](i), &operator[](i + 1), (m_size - i - 1));

    --m_size;
}

template<typename T>
inline isize
VecBase<T>::idx(const T* const x) const noexcept
{
    isize r = isize(x - m_pData);
    ADT_ASSERT(r >= 0 && r < m_cap,"r: {}, cap: {}, addr: {}. Must take the address of the reference", r, m_cap, (void*)x);
    return r;
}

template<typename T>
inline isize
VecBase<T>::lastI() const noexcept
{
    return idx(&last());
}

template<typename T>
inline void
VecBase<T>::destroy(IAllocator* p) noexcept
{
    p->dealloc(m_pData, m_size);
    *this = {};
}

template<typename T>
inline VecBase<T>
VecBase<T>::release() noexcept
{
    return utils::exchange(this, {});
}

template<typename T>
inline isize
VecBase<T>::size() const noexcept
{
    return m_size;
}

template<typename T>
inline isize
VecBase<T>::cap() const noexcept
{
    return m_cap;
}

template<typename T>
inline T*
VecBase<T>::data() noexcept
{
    return m_pData;
}

template<typename T>
inline const T*
VecBase<T>::data() const noexcept
{
    return m_pData;
}

template<typename T>
inline void
VecBase<T>::zeroOut() noexcept
{
    utils::memSet(m_pData, 0, m_size);
}

template<typename T>
inline VecBase<T>
VecBase<T>::clone(IAllocator* pAlloc) const
{
    auto nVec = VecBase<T>(pAlloc, m_cap);
    utils::memCopy(nVec.data(), m_pData, m_size);
    nVec.m_size = m_size;

    return nVec;
}

template<typename T>
inline bool
VecBase<T>::search(const T& x) const
{
    for (const auto& el : *this)
        if (el == x) return true;

    return false;
}

template<typename T>
inline void
VecBase<T>::grow(IAllocator* p, isize newCapacity)
{
    m_pData = p->relocate<T>(m_pData, m_cap, newCapacity);
    m_cap = newCapacity;
}

template<typename T>
inline void
VecBase<T>::growIfNeeded(IAllocator* p)
{
    if (m_size >= m_cap)
    {
        const isize newCap = (m_cap+1) * 2;
        ADT_ASSERT(newCap > m_cap, "can't grow (capacity overflow), newCap: {}, m_capacity: {}", newCap, m_cap);
        grow(p, newCap);
    }
}

template<typename T>
inline void
VecBase<T>::growOnSpanPush(IAllocator* p, const isize spanSize)
{
    ADT_ASSERT(spanSize > 0, "pushing empty span");
    ADT_ASSERT(m_size + spanSize >= m_size, "overflow");

    if (m_size + spanSize > m_cap)
    {
        const isize newSize = (m_size + spanSize + 1) * 2;
        ADT_ASSERT(newSize > m_size, "overflow");
        grow(p, newSize);
    }
}

template<typename T, typename ALLOC_T = StdAllocatorNV>
struct VecManaged : ALLOC_T, VecBase<T>
{
    using Base = VecBase<T>;
    using Base::m_pData;
    using Base::m_size;
    using Base::m_cap;

    /* */

    VecManaged() = default;
    VecManaged(UninitFlag) noexcept {}
    VecManaged(const isize prealloc) : Base{allocator(), prealloc} {}
    VecManaged(const isize prealloc, const T& fillWith) : Base{allocator(), prealloc, fillWith} {}

    ~VecManaged() noexcept { if (allocator()) Base::destroy(allocator()); }

    VecManaged(VecManaged&&) noexcept;
    VecManaged& operator=(VecManaged&&) noexcept;

    VecManaged(const VecManaged&);
    VecManaged& operator=(const VecManaged&);

protected:
    VecManaged(IAllocator* pAlloc, const isize prealloc) : ALLOC_T{pAlloc}, Base{allocator(), prealloc} {}
    VecManaged(IAllocator* pAlloc, const isize prealloc, const T& fillWith) : ALLOC_T{pAlloc}, Base{allocator(), prealloc, fillWith} {}
public:

    /* */

    auto* allocator() const noexcept { return ALLOC_T::ptr(); }

    isize fakePush() { return Base::fakePush(allocator()); }

    isize push(const T& data) { return Base::push(allocator(), data); }
    isize push(T&& data) { return Base::push(allocator(), std::move(data)); }

    void pushAt(const isize atI, const T& data) { Base::pushAt(allocator(), atI, data); }
    void pushAt(const isize atI, T&& data) { Base::pushAt(allocator(), atI, std::move(data)); }

    isize pushSpan(const Span<const T> sp) { return Base::pushSpan(allocator(), sp); }

    void pushSpanAt(const isize atI, const Span<const T> sp) { Base::pushSpanAt(allocator(), atI, sp); }

    template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
    isize emplace(ARGS&&... args) { return Base::emplace(allocator(), std::forward<ARGS>(args)...); }

    template<typename ...ARGS> requires(std::is_constructible_v<T, ARGS...>)
    void emplaceAt(const isize atI, ARGS&&... args) { Base::emplaceAt(allocator(), atI, std::forward<ARGS>(args)...); }

    void setSize(isize size) { Base::setSize(allocator(), size); }

    void setCap(isize cap) { Base::setCap(allocator(), cap); }

    void destroy() noexcept { Base::destroy(allocator()); }
};

template<typename T, typename ALLOC_T>
inline
VecManaged<T, ALLOC_T>::VecManaged(VecManaged&& x) noexcept
    : Base{UNINIT}
{
    m_pData = utils::exchange(&x.m_pData, nullptr);
    m_size = utils::exchange(&x.m_size, 0);
    m_cap = utils::exchange(&x.m_cap, 0);

    if constexpr (std::is_same_v<ALLOC_T, IAllocatorMember>)
    {
        static_cast<ALLOC_T*>(this)->m_pAlloc = x.allocator();
        static_cast<ALLOC_T*>(&x)->m_pAlloc = nullptr;
    }
}

template<typename T, typename ALLOC_T>
inline VecManaged<T, ALLOC_T>&
VecManaged<T, ALLOC_T>::operator=(VecManaged&& x) noexcept
{
    if (this != &x)
    {
        m_pData = utils::exchange(&x.m_pData, nullptr);
        m_size = utils::exchange(&x.m_size, 0);
        m_cap = utils::exchange(&x.m_cap, 0);

        if constexpr (std::is_same_v<ALLOC_T, IAllocatorMember>)
            ALLOC_T::m_pAlloc = utils::exchange(&x.m_pAlloc, nullptr);
    }

    return *this;
}

template<typename T, typename ALLOC_T>
inline
VecManaged<T, ALLOC_T>::VecManaged(const VecManaged& x)
    : Base{x.allocator(), x.m_size}
{
    if constexpr (std::is_same_v<ALLOC_T, IAllocatorMember>)
        ALLOC_T::m_pAlloc = x.m_pAlloc;

    setSize(x.m_size);
    utils::memCopy(m_pData, x.m_pData, m_size);
}

template<typename T, typename ALLOC_T>
inline VecManaged<T, ALLOC_T>&
VecManaged<T, ALLOC_T>::operator=(const VecManaged& x)
{
    if (this != &x)
    {
        if constexpr (std::is_same_v<ALLOC_T, IAllocatorMember>)
            if (!allocator()) ALLOC_T::m_pAlloc = x.m_pAlloc;

        setSize(x.m_size);
        utils::memCopy(m_pData, x.m_pData, m_size);
    }

    return *this;
}

template<typename T>
using VecM = VecManaged<T, StdAllocatorNV>;

template<typename T>
struct Vec: VecManaged<T, IAllocatorMember>
{
    using Base = VecManaged<T, IAllocatorMember>;

    /* */

    Vec() = default;
    Vec(IAllocator* pAlloc, const isize prealloc = 8 / sizeof(T)) : Base{pAlloc, prealloc} {}
    Vec(IAllocator* pAlloc, const isize prealloc, const T& fillWith) : Base{pAlloc, prealloc, fillWith} {}
};

} /* namespace adt */
