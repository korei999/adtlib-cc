#include "String-inl.hh"

namespace adt::utf8
{

inline isize
prevI(const StringView sv, isize pos) noexcept
{
    ADT_ASSERT(pos >= 0 && pos < sv.m_size, "pos: {}, size: {}", pos, sv.m_size);

    isize i = pos - 1;
    while (i > 0 && ((u8)(sv.m_pData[i]) & 0xc0) == 0x80) --i;
    return i;
}

} /* adt::utf8 */
