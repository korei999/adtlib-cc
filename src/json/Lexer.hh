#pragma once

#include "adt/String.hh"

namespace json
{

enum RESULT : adt::u8 { OK, FAIL };

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
    adt::u32 raw {};
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
    Lexer(adt::String sJson) : m_sJson(sJson) {}

    /* */

    Token next();
    bool done() const { return m_pos >= m_sJson.getSize(); }

    /* */

private:
    void skipWhitespace();
    Token nextString();
    Token nextStringNoQuotes();
    Token nextNumber();

    void advance(std::size_t nChars) { m_pos += nChars, m_column += nChars; }
};

} /* namespace json */
