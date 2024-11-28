#pragma once

#include "Vec.hh"
#include "hash.hh"

namespace adt
{

constexpr f32 MAP_DEFAULT_LOAD_FACTOR = 0.5f;
constexpr f32 MAP_DEFAULT_LOAD_FACTOR_INV = 1.0f / MAP_DEFAULT_LOAD_FACTOR;

template<typename T>
struct MapBucket
{
    T data;
    bool bOccupied = false;
    bool bDeleted = false;
};

/* custom return type for insert/search operations */
template<typename T>
struct MapResult
{
    T* pData;
    u64 hash;
    bool bInserted;

    constexpr explicit
    operator bool() const
    {
        return this->pData != nullptr;
    }
};

template<typename T> struct MapBase;

template<typename T> inline void _MapRehash(MapBase<T>* s, Allocator* p, u32 size);


/* Value is the key in this map.
 * For key/value pairs, use struct { KEY k; VALUE v; }, add `u64 adt::hash::func(const struct&)`
 * and* bool operator==(const struct& l, const struct& r). */
template<typename T>
struct MapBase
{
    VecBase<MapBucket<T>> aBuckets {};
    f32 maxLoadFactor {};
    u32 bucketCount {};

    MapBase() = default;
    MapBase(Allocator* pAllocator, u32 prealloc = SIZE_MIN);

    u32 firstI();
    u32 idx(T* p);
    u32 idx(MapResult<T>* pRes);
    u32 nextI(u32 i);
    f32 loadFactor();
    MapResult<T> insert(Allocator* p, const T& value);
    MapResult<T> search(const T& value);
    void remove(u32 i);
    void remove(const T& x);
    void _rehash(Allocator* p, u32 size);
    MapResult<T> tryInsert(Allocator* p, const T& value);
    void destroy(Allocator* p);

    struct It
    {
        MapBase* s {};
        u32 i = 0;

        It(MapBase* _s, u32 _i) : s(_s), i(_i) {}

        T& operator*() { return s->aBuckets[i].data; }
        T* operator->() { return &s->aBuckets[i].data; }

        It operator++()
        {
            i = MapNextI(s, i);
            return {s, i};
        }
        It operator++(int) { T* tmp = s++; return tmp; }

        friend constexpr bool operator==(const It& l, const It& r) { return l.i == r.i; }
        friend constexpr bool operator!=(const It& l, const It& r) { return l.i != r.i; }
    };

    It begin() { return {this, MapFirstI(this)}; }
    It end() { return {this, NPOS}; }

    const It begin() const { return {this, MapFirstI(this)}; }
    const It end() const { return {this, NPOS}; }
};

template<typename T>
inline u32
MapBase<T>::idx(T* p)
{
    return p - this->aBuckets.data();
}

template<typename T>
inline u32
MapBase<T>::idx(MapResult<T>* pRes)
{
    return pRes->pData - this->aBuckets.data();
}

template<typename T>
inline u32
MapBase<T>::firstI()
{
    u32 i = 0;
    while (i < this->aBuckets.getCap() && !this->aBuckets[i].bOccupied)
        i++;

    if (i >= this->aBuckets.getCap()) i = NPOS;

    return i;
}

template<typename T>
inline u32
MapBase<T>::nextI(u32 i)
{
    ++i;
    while (i < this->aBuckets.getCap() && !this->aBuckets[i].bOccupied)
        i++;

    if (i >= this->aBuckets.getCap()) i = NPOS;

    return i;
}

template<typename T>
inline f32
MapBase<T>::loadFactor()
{
    return f32(this->bucketCount) / f32(this->aBuckets.getCap());
}

template<typename T>
inline MapResult<T>
MapBase<T>::insert(Allocator* p, const T& value)
{
    if (this->aBuckets.getCap() == 0) *this = {p};

    if (this->loadFactor() >= this->maxLoadFactor)
        this->_rehash(p, this->aBuckets.getCap() * 2);

    u64 hash = hash::func(value);
    u32 idx = u32(hash % this->aBuckets.getCap());

    while (this->aBuckets[idx].bOccupied)
    {
        idx++;
        if (idx >= this->aBuckets.getCap())
            idx = 0;
    }

    this->aBuckets[idx].data = value;
    this->aBuckets[idx].bOccupied = true;
    this->aBuckets[idx].bDeleted = false;
    this->bucketCount++;

    return {
        .pData = &this->aBuckets[idx].data,
        .hash = hash,
        .bInserted = true
    };
}

template<typename T>
inline MapResult<T>
MapBase<T>::search(const T& value)
{
    if (this->bucketCount == 0) return {};

    u64 hash = hash::func(value);
    u32 idx = u32(hash % this->aBuckets.getCap());

    MapResult<T> ret;
    ret.hash = hash;
    ret.pData = nullptr;
    ret.bInserted = false;

    while (this->aBuckets[idx].bOccupied || this->aBuckets[idx].bDeleted)
    {
        if (this->aBuckets[idx].data == value)
        {
            ret.pData = &this->aBuckets[idx].data;
            break;
        }

        idx++;
        if (idx >= this->aBuckets.getCap()) idx = 0;
    }

    return ret;
}

template<typename T>
inline void
MapBase<T>::remove(u32 i)
{
    this->aBuckets[i].bDeleted = true;
    this->aBuckets[i].bOccupied = false;

    this->bucketCount--;
}

template<typename T>
inline void
MapBase<T>::remove(const T& x)
{
    auto f = this->search(x);
    auto idx = this->idx(f.pData);
    this->remove(idx);
}

template<typename T>
inline void
MapBase<T>::_rehash(Allocator* p, u32 size)
{
    auto mNew = MapBase<T>(p, size);

    for (u32 i = 0; i < this->aBuckets.getCap(); i++)
        if (this->aBuckets[i].bOccupied)
            mNew.insert(p, this->aBuckets[i].data);

    this->destroy(p);
    *this = mNew;
}

template<typename T>
inline MapResult<T>
MapBase<T>::tryInsert(Allocator* p, const T& value)
{
    auto f = this->search(value);
    if (f) return f;
    else return this->insert(p, value);
}

template<typename T>
inline void
MapBase<T>::destroy(Allocator* p)
{
    this->aBuckets.destroy(p);
}

template<typename T>
MapBase<T>::MapBase(Allocator* pAllocator, u32 prealloc)
    : aBuckets(pAllocator, prealloc * MAP_DEFAULT_LOAD_FACTOR_INV),
      maxLoadFactor(MAP_DEFAULT_LOAD_FACTOR)
{
    this->aBuckets.setSize(pAllocator, prealloc * MAP_DEFAULT_LOAD_FACTOR_INV);
    memset(aBuckets.pData, 0, sizeof(aBuckets[0]) * aBuckets.getSize());
}

template<typename T>
struct Map
{
    MapBase<T> base {};
    Allocator* pA {};

    Map() = default;
    Map(Allocator* _pA, u32 prealloc = SIZE_MIN)
        : base(_pA, prealloc), pA(_pA) {}

    u32 firstI() { return base.firstI(); }
    u32 idx(T* p) { return base.idx(p); }
    u32 idx(MapResult<T>* pRes) { return base.idx(pRes); }
    u32 nextI(u32 i) { return base.nextI(i); }
    f32 loadFactor() { return base.loadFactor(); }
    MapResult<T> insert(const T& value) { return base.insert(pA, value); }
    MapResult<T> search(const T& value) { return base.search(value); }
    void remove(u32 i) { base.remove(i); }
    void remove(const T& x) { base.remove(x); }
    void _rehash(u32 size) { base._rehash(pA, size); }
    MapResult<T> tryInsert(const T& value) { return base.tryInsert(pA, value); }
    void destroy() { base.destroy(pA); }

    MapBase<T>::It begin() { return base.begin(); }
    MapBase<T>::It end() { return base.end(); }

    const MapBase<T>::It begin() const { return base.begin(); }
    const MapBase<T>::It end() const { return base.end(); }
};

} /* namespace adt */
