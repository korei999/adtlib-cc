#pragma once

#include "Vec.hh"
#include "hash.hh"

namespace adt
{

constexpr f32 MAP_DEFAULT_LOAD_FACTOR = 0.5f;
constexpr f32 MAP_DEFAULT_LOAD_FACTOR_INV = 1.0f / MAP_DEFAULT_LOAD_FACTOR;

enum class MAP_RESULT_STATUS : u8 { FOUND, NOT_FOUND, INSERTED };

template<typename K, typename V>
struct KeyValue
{
    K key {};
    V val {};
};

template<typename K, typename V, template<typename, typename> typename CON_T>
struct MapBucket
{
    CON_T<K, V> keyVal {};
    bool bOccupied = false;
    bool bDeleted = false;
};

/* custom return type for insert/search operations */
template<typename K, typename V, template<typename, typename> typename CON_T>
struct MapResult
{
    CON_T<K, V>* pData {};
    u64 hash {};
    MAP_RESULT_STATUS eStatus {};

    constexpr explicit operator bool() const
    {
        return this->pData != nullptr;
    }
};


template<typename K, typename V, template<typename, typename> typename CON_T = KeyValue> struct MapBase;

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapIdx(MapBase<K, V, C>* s, C<K, V>* p);

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapIdx(MapBase<K, V, C>* s, MapResult<K, V, C> res);

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapFirstI(MapBase<K, V, C>* s);

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapNextI(MapBase<K, V, C>* s, u32 i);

template<typename K, typename V, template<typename, typename> typename C>
inline f32 MapLoadFactor(MapBase<K, V, C>* s);

template<typename K, typename V, template<typename, typename> typename C>
inline MapResult<K, V, C> MapInsert(MapBase<K ,V, C>* s, Allocator* p, const K& key, const V& val);

template<typename K, typename V, template<typename, typename> typename C>
[[nodiscard]] inline MapResult<K, V, C> MapSearch(MapBase<K, V, C>* s, const K& key);

template<typename K, typename V, template<typename, typename> typename C>
inline void MapRemove(MapBase<K, V, C>*s, u32 i);

template<typename K, typename V, template<typename, typename> typename C>
inline void MapRemove(MapBase<K, V, C>*s, const K& key);

template<typename K, typename V, template<typename, typename> typename C>
inline MapResult<K, V, C> MapTryInsert(MapBase<K, V, C>* s, Allocator* p, const K& key, const V& value);

template<typename K, typename V, template<typename, typename> typename C>
inline void MapDestroy(MapBase<K, V, C>* s, Allocator* p);

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapCap(MapBase<K, V, C>* s);

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapSize(MapBase<K, V, C>* s);

template<typename K, typename V, template<typename, typename> typename C>
inline void _MapRehash(MapBase<K, V, C>* s, Allocator* p, u32 size);

template<typename K, typename V, template<typename, typename> typename CON_T>
struct MapBase
{
    VecBase<MapBucket<K, V, CON_T>> aBuckets {};
    f32 maxLoadFactor {};
    u32 nOccupied {};

    MapBase() = default;
    MapBase(Allocator* pAllocator, u32 prealloc = SIZE_MIN);

    struct It
    {
        MapBase* s {};
        u32 i = 0;

        It(MapBase* _s, u32 _i) : s(_s), i(_i) {}

        CON_T<K, V>& operator*() { return s->aBuckets[i].keyVal; }
        CON_T<K, V>* operator->() { return &s->aBuckets[i].keyVal; }

        It operator++()
        {
            i = MapNextI(s, i);
            return {s, i};
        }
        It operator++(int) { CON_T<K, V>* tmp = s++; return tmp; }

        friend bool operator==(const It& l, const It& r) { return l.i == r.i; }
        friend bool operator!=(const It& l, const It& r) { return l.i != r.i; }
    };

    It begin() { return {this, MapFirstI(this)}; }
    It end() { return {this, NPOS}; }

    const It begin() const { return {this, MapFirstI(this)}; }
    const It end() const { return {this, NPOS}; }
};

template<typename K, typename V, template<typename, typename> typename C>
inline u32
MapIdx(MapBase<K, V, C>* s, C<K, V>* p)
{
    auto r = (MapBucket<K, V, C>*)p - &s->aBuckets[0];
    assert(r < VecCap(&s->aBuckets));
    return r;
}

template<typename K, typename V, template<typename, typename> typename C>
inline u32
MapIdx(MapBase<K, V, C>* s, MapResult<K, V, C> res)
{
    return MapIdx<K, V, C>(s, res.pData);
}

template<typename K, typename V, template<typename, typename> typename C>
inline u32
MapFirstI(MapBase<K, V, C>* s)
{
    u32 i = 0;
    while (i < VecCap(&s->aBuckets) && !s->aBuckets[i].bOccupied)
        ++i;

    if (i >= VecCap(&s->aBuckets)) i = NPOS;

    return i;
}

template<typename K, typename V, template<typename, typename> typename C>
inline u32
MapNextI(MapBase<K, V, C>* s, u32 i)
{
    do ++i;
    while (i < VecCap(&s->aBuckets) && !s->aBuckets[i].bOccupied);

    if (i >= VecCap(&s->aBuckets)) i = NPOS;

    return i;
}

template<typename K, typename V, template<typename, typename> typename C>
inline f32
MapLoadFactor(MapBase<K, V, C>* s)
{
    return f32(s->nOccupied) / f32(VecCap(&s->aBuckets));
}

template<typename K, typename V, template<typename, typename> typename C>
inline MapResult<K, V, C>
MapInsert(MapBase<K ,V, C>* s, Allocator* p, const K& key, const V& value)
{
    if (VecCap(&s->aBuckets) == 0) *s = {p};

    if (MapLoadFactor(s) >= s->maxLoadFactor)
        _MapRehash(s, p, VecCap(&s->aBuckets) * 2);

    u64 hash = hash::func(key);
    u32 idx = u32(hash % VecCap(&s->aBuckets));

    while (s->aBuckets[idx].bOccupied)
    {
        ++idx;
        if (idx >= VecCap(&s->aBuckets)) idx = 0;
    }

    s->aBuckets[idx].keyVal = {key, value};
    s->aBuckets[idx].bOccupied = true;
    s->aBuckets[idx].bDeleted = false;
    ++s->nOccupied;

    return {
        .pData = &s->aBuckets[idx].keyVal,
        .hash = hash,
        .eStatus = MAP_RESULT_STATUS::INSERTED
    };
}

template<typename K, typename V, template<typename, typename> typename C>
[[nodiscard]] inline MapResult<K, V, C>
MapSearch(MapBase<K, V, C>* s, const K& key)
{
    MapResult<K, V, C> res {.eStatus = MAP_RESULT_STATUS::NOT_FOUND};

    if (s->nOccupied == 0) return res;

    auto& aBuckets = s->aBuckets;

    u64 hash = hash::func(key);
    u32 idx = u32(hash % VecCap(&aBuckets));
    res.hash = hash;

    while (aBuckets[idx].bOccupied || aBuckets[idx].bDeleted)
    {
        const auto& [k, v] = aBuckets[idx].keyVal;
        if (!aBuckets[idx].bDeleted && k == key)
        {
            res.pData = &aBuckets[idx].keyVal;
            res.eStatus = MAP_RESULT_STATUS::FOUND;
            break;
        }

        ++idx;
        if (idx >= VecCap(&aBuckets)) idx = 0;
    }

    return res;
}

template<typename K, typename V, template<typename, typename> typename C>
inline void
MapRemove(MapBase<K, V, C>*s, u32 i)
{
    s->aBuckets[i].bDeleted = true;
    s->aBuckets[i].bOccupied = false;
    --s->nOccupied;
}

template<typename K, typename V, template<typename, typename> typename C>
inline void
MapRemove(MapBase<K, V, C>*s, const K& key)
{
    auto f = MapSearch<K, V, C>(s, key);
    assert(f && "[Map]: not found");
    MapRemove<K, V, C>(s, MapIdx<K, V, C>(s, f));
}


template<typename K, typename V, template<typename, typename> typename C>
inline MapResult<K, V, C>
MapTryInsert(MapBase<K, V, C>* s, Allocator* p, const K& key, const V& value)
{
    auto f = MapSearch<K, V, C>(s, key);
    if (f)
    {
        f.eStatus = MAP_RESULT_STATUS::FOUND;
        return f;
    }
    else return MapInsert<K, V, C>(s, p, key, value);
}

template<typename K, typename V, template<typename, typename> typename C>
inline void
MapDestroy(MapBase<K, V, C>* s, Allocator* p)
{
    VecDestroy(&s->aBuckets, p);
}

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapCap(MapBase<K, V, C>* s)
{
    return VecCap(&s->aBuckets);
}

template<typename K, typename V, template<typename, typename> typename C>
inline u32 MapSize(MapBase<K, V, C>* s)
{
    return s->nOccupied;
}

template<typename K, typename V, template<typename, typename> typename C>
inline void
_MapRehash(MapBase<K, V, C>* s, Allocator* p, u32 size)
{
    auto mNew = MapBase<K, V, C>(p, size);

    for (u32 i = 0; i < VecCap(&s->aBuckets); ++i)
        if (s->aBuckets[i].bOccupied)
        {
            const auto& [key, val] = s->aBuckets[i].keyVal;
            MapInsert<K, V, C>(&mNew, p, key, val);
        }

    MapDestroy(s, p);
    *s = mNew;
}

template<typename K, typename V, template<typename, typename> typename C>
MapBase<K, V, C>::MapBase(Allocator* pAllocator, u32 prealloc)
    : aBuckets(pAllocator, prealloc * MAP_DEFAULT_LOAD_FACTOR_INV),
      maxLoadFactor(MAP_DEFAULT_LOAD_FACTOR)
{
    VecSetSize(&aBuckets, pAllocator, prealloc * MAP_DEFAULT_LOAD_FACTOR_INV);
    memset(aBuckets.pData, 0, sizeof(aBuckets[0]) * VecSize(&aBuckets));
}

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
struct Map
{
    MapBase<K, V, C> base {};
    Allocator* pA {};

    Map() = default;
    Map(Allocator* _pA, u32 prealloc = SIZE_MIN)
        : base(_pA, prealloc), pA(_pA) {}

    MapBase<K, V, C>::It begin() { return base.begin(); }
    MapBase<K, V, C>::It end() { return base.end(); }
    MapBase<K, V, C>::It rbegin() { return base.rbegin(); }
    MapBase<K, V, C>::It rend() { return rend(); }

    const MapBase<K, V, C>::It begin() const { return base.begin(); }
    const MapBase<K, V, C>::It end() const { return base.end(); }
    const MapBase<K, V, C>::It rbegin() const { return base.rbegin(); }
    const MapBase<K, V, C>::It rend() const { return rend(); }
};

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapIdx(Map<K, V, C>* s, C<K, V>* pItem) { return MapIdx<K, V, C>(&s->base, pItem); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapIdx(Map<K, V, C>* s, MapResult<K, V, C> res) { return MapIdx<K, V, C>(&s->base, res); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapFirstI(Map<K, V, C>* s) { return MapFirstI<K, V, C>(&s->base); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapNextI(Map<K, V, C>* s, u32 i) { return MapNextI<K, V, C>(&s->base, i); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline f32 MapLoadFactor(Map<K, V, C>* s) { return MapLoadFactor<K, V, C>(&s->base); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline MapResult<K, V, C> MapInsert(Map<K, V, C>* s, const K& key, const V& value) { return MapInsert<K, V, C>(&s->base, s->pA, key, value); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
[[nodiscard]] inline MapResult<K, V, C> MapSearch(Map<K, V, C>* s, const K& key) { return MapSearch<K, V, C>(&s->base, key); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline void MapRemove(Map<K, V, C>*s, u32 i) { MapRemove<K, V, C>(&s->base, i); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline void MapRemove(Map<K, V, C>*s, const K& key) { MapRemove<K, V, C>(&s->base, key); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline MapResult<K, V, C> MapTryInsert(Map<K, V, C>* s, const K& key, const V& value) { return MapTryInsert<K, V, C>(&s->base, s->pA, key, value); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline void MapDestroy(Map<K, V, C>* s) { MapDestroy<K, V, C>(&s->base, s->pA); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapCap(Map<K, V, C>* s) { return MapCap<K, V, C>(&s->base); }

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32 MapSize(Map<K, V, C>* s) { return MapSize<K, V, C>(&s->base); }

namespace print
{

inline u32
formatToContext(Context ctx, [[maybe_unused]] FormatArgs fmtArgs, MAP_RESULT_STATUS eStatus)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;
    constexpr String map[] {
        "FOUND", "NOT_FOUND", "INSERTED"
    };

    auto statusIdx = std::underlying_type_t<MAP_RESULT_STATUS>(eStatus);
    assert(statusIdx < utils::size(map) && "out of range enum");
    return printArgs(ctx, map[statusIdx]);
}

template<typename K, typename V, template<typename, typename> typename C = KeyValue>
inline u32
formatToContext(Context ctx, [[maybe_unused]] FormatArgs fmtArgs, const C<K, V>& x)
{
    ctx.fmt = "[{}, {}]";
    ctx.fmtIdx = 0;
    const auto& [k, v] = x;
    return printArgs(ctx, k, v);
}

} /* namespace print */

} /* namespace adt */
