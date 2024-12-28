#pragma once

#include "adt/String.hh"
#include "adt/Vec.hh"

namespace json
{

enum class TAG : adt::u8
{
    NULL_,
    STRING,
    LONG,
    DOUBLE,
    ARRAY,
    OBJECT,
    BOOL
};

static const char* TAGStrings[] {
    "NULL_", "STRING", "LONG", "DOUBLE", "ARRAY", "OBJECT", "BOOL"
};

inline const char*
getTAGString(enum TAG t)
{
    return TAGStrings[static_cast<int>(t)];
}

struct Object;

union Val
{
    adt::null n;
    adt::String sv;
    adt::s64 l;
    adt::f64 d;
    adt::VecBase<Object> a;
    adt::VecBase<Object> o;
    bool b;
};

struct TagVal
{
    enum TAG tag;
    union Val val;
};

struct Object
{
    adt::String svKey;
    TagVal tagVal;
};

} /* namespace json */
