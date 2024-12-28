#pragma once

#include "adt/print.hh"

namespace json
{

enum class TOKEN_TYPE : adt::u8
{
    NONE,
    DOT,
    COMMA,
    STRING,
    QUOTED_STRING,
    COLON,
    L_BRACE,
    R_BRACE,
    L_BRACKET,
    R_BRACKET,
    NUMBER,
    FLOAT,
};

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

    constexpr String map[] {
        "NONE",
        "DOT",
        "COMMA",
        "STRING",
        "QUOTED_STRING",
        "COLON",
        "L_BRACE",
        "R_BRACE",
        "L_BRACKET",
        "R_BRACKET",
        "NUMBER",
        "FLOAT",
    };

    return printArgs(ctx, map[int(x)]);
}

} /* namespace print */
} /* namespace adt */
