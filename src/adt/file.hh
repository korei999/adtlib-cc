#pragma once

#include "String.hh"
#include "Array.hh"

#include <stdio.h>

namespace adt
{

inline String
loadFile(Allocator* pAlloc, String path)
{
    String ret;

    auto sn = StringCreate(pAlloc, path);

    FILE* pf = fopen(sn.pData, "rb");
    if (pf)
    {
        fseek(pf, 0, SEEK_END);
        long size = ftell(pf) + 1;
        rewind(pf);

        ret.pData = (char*)alloc(pAlloc, size, sizeof(char));
        ret.size = size - 1;
        fread(ret.pData, 1, ret.size, pf);

        fclose(pf);
    }

    return ret;
}

inline Array<u8>
loadFileToCharArray(Allocator* pAlloc, String path)
{
    Array<u8> ret(pAlloc);

    FILE* pf = fopen(path.pData, "rb");
    if (pf)
    {
        fseek(pf, 0, SEEK_END);
        long size = ftell(pf);
        rewind(pf);

        ArraySetSize(&ret, size + 1);
        ret.size = size;
        fread(ret.pData, 1, size, pf);

        fclose(pf);
    }

    return ret;
}

inline String
replacePathEnding(Allocator* pAlloc, adt::String path, adt::String sEnding)
{
    auto lastSlash = adt::StringLastOf(path, '/');
    adt::String sNoEnding = {&path[0], lastSlash + 1};
    auto r = adt::StringCat(pAlloc, sNoEnding, sEnding);
    return r;
}

} /* namespace adt */
