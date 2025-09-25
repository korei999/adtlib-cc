#pragma once

#include "String-inl.hh"

#include <source_location>
#include <exception>

#define ADT_RUNTIME_EXCEPTION(CND)                                                                                     \
    if (!static_cast<bool>(CND))                                                                                       \
        throw adt::RuntimeException(#CND);

#define ADT_RUNTIME_EXCEPTION_FMT(CND, ...)                                                                            \
    if (!static_cast<bool>(CND))                                                                                       \
    {                                                                                                                  \
        adt::RuntimeException ex;                                                                                      \
        auto& aMsgBuff = ex.m_sfMsg.data();                                                                            \
        adt::isize n = adt::print::toBuffer(aMsgBuff, sizeof(aMsgBuff) - 1, #CND);                                     \
        n += adt::print::toBuffer(aMsgBuff + n, sizeof(aMsgBuff) - 1 - n, "\nMsg: ");                                  \
        n += adt::print::toBuffer(aMsgBuff + n, sizeof(aMsgBuff) - 1 - n, __VA_ARGS__);                                \
        throw ex;                                                                                                      \
    }

namespace adt
{

struct IException : std::exception
{
    IException() = default;
    virtual ~IException() = default;
};

struct RuntimeException : public IException
{
    StringFixed<256> m_sfMsg {};
    std::source_location m_loc {};


    /* */

    RuntimeException(std::source_location loc = std::source_location::current()) : m_sfMsg {}, m_loc {loc} {}
    RuntimeException(const StringView svMsg, std::source_location loc = std::source_location::current())
        : m_sfMsg(svMsg), m_loc {loc} {}

    virtual ~RuntimeException() = default;

    /* */

    virtual const char*
    what() const noexcept override
    {
        return m_sfMsg.data();
    }
};

} /* namespace adt */
