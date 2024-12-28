#pragma once

#include "adt/print.hh"
#include "adt/enum.hh"

namespace json
{

enum class TOKEN_TYPE : adt::u32
{
    NONE = 0,
    DOT = 1,
    COMMA = 1 << 1,
    STRING = 1 << 2,
    QUOTED_STRING = 1 << 3,
    COLON = 1 << 4,
    L_BRACE = 1 << 5,
    R_BRACE = 1 << 6,
    L_BRACKET = 1 << 7,
    R_BRACKET = 1 << 8,
    NUMBER = 1 << 9,
    FLOAT = 1 << 10,
};
ADT_ENUM_BITWISE_OPERATORS(TOKEN_TYPE);

struct Token
{
    TOKEN_TYPE eType {};
    adt::String sLiteral {};
    adt::u32 row {};
    adt::u32 column {};
};

class Lexer
{
    adt::String m_sJson {};
    adt::u32 m_pos {};
    adt::u32 m_row = 1;
    adt::u32 m_column = 1;

    /* */

public:
    Lexer() = default;
    Lexer(adt::String sJson) : m_sJson(sJson), m_row(1), m_column(1) {}

    /* */

    Token next();
    bool done() const { return m_pos >= m_sJson.getSize(); }

    /* */

private:
    void skipWhitespace();
    Token nextChar(TOKEN_TYPE eType);
    Token nextString();
    Token nextStringNoQuotes();
    Token nextNumber();

    void advanceOne() { ++m_pos, ++m_column; }
};

} /* namespace json */

namespace adt
{
namespace print
{

inline u32
formatToContext(Context ctx, [[maybe_unused]]  FormatArgs fmtArgs, const json::TOKEN_TYPE& x)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;

    const char* nts = "UNKNOWN";
    switch (x)
    {
        case json::TOKEN_TYPE::NONE: nts = "NONE"; break;
        case json::TOKEN_TYPE::DOT: nts = "DOT"; break;
        case json::TOKEN_TYPE::COMMA: nts = "COMMA"; break;
        case json::TOKEN_TYPE::STRING: nts = "STRING"; break;
        case json::TOKEN_TYPE::QUOTED_STRING: nts = "QUOTED_STRING"; break;
        case json::TOKEN_TYPE::COLON: nts = "COLON"; break;
        case json::TOKEN_TYPE::L_BRACE: nts = "L_BRACE"; break;
        case json::TOKEN_TYPE::R_BRACE: nts = "R_BRACE"; break;
        case json::TOKEN_TYPE::L_BRACKET: nts = "L_BRACKET"; break;
        case json::TOKEN_TYPE::R_BRACKET: nts = "R_BRACKET"; break;
        case json::TOKEN_TYPE::NUMBER: nts = "NUMBER"; break;
        case json::TOKEN_TYPE::FLOAT: nts = "FLOAT"; break;
    }

    return printArgs(ctx, nts);
}

} /* namespace print */
} /* namespace adt */
