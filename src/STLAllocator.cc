#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <vector>
#include <memory_resource>

#include "adt/Logger.hh"
#include "adt/Arena.hh"

template<class T>
struct Mallocator
{
    using value_type = T;
 
    Mallocator() = default;
 
    template<class U>
    constexpr Mallocator(const Mallocator <U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n)
    {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();
 
        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
        {
            report(p, n);
            return p;
        }
 
        throw std::bad_alloc();
    }
 
    void deallocate(T* p, std::size_t n) noexcept
    {
        report(p, n, 0);
        std::free(p);
    }

    template<class U> constexpr bool operator==(const Mallocator <U>&) const { return true; }
    template<class U> constexpr bool operator!=(const Mallocator <U>&) const { return false; }

private:
    void report(T* p, std::size_t n, bool alloc = true) const
    {
        std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
                  << " bytes at " << std::hex << std::showbase
                  << reinterpret_cast<void*>(p) << std::dec << '\n';
    }
};

class ArenaPmr : public std::pmr::memory_resource
{
    adt::Arena m_arena {};

    /* */

public:
    ArenaPmr() noexcept = default;
    ArenaPmr(size_t reserve, size_t commit = adt::getPageSize()) : m_arena{(adt::isize)reserve, (adt::isize)commit} {}

    virtual ~ArenaPmr() noexcept override { m_arena.freeAll(); }

protected:
    virtual void*
    do_allocate(size_t __bytes, size_t __alignment) override
    {
        return m_arena.malloc(1, __bytes);
    }

    virtual void
    do_deallocate(void*, size_t __bytes, size_t __alignment) override
    {
        /* noop */
        adt::print::out("do_deallocate: __bytes: {}, __alignment: {}\n", __bytes, __alignment);
    }

    virtual bool
    do_is_equal(const memory_resource& __other) const noexcept override
    {
        return this == &__other;
    }
};

int
main()
{
    try
    {
        ArenaPmr arena {adt::SIZE_8G};

        std::pmr::vector<adt::Pair<short, short>> v {&arena};

        v.push_back({1, 2});
        v.push_back({1, 2});
        v.push_back({1, 2});
        v.push_back({1, 2});

        adt::print::out("v: {}\n", v);
    }
    catch (std::exception& ex)
    {
        adt::print::out("exception: '{}'\n", ex.what());
    }
}
