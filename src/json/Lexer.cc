#include "Lexer.hh"

#include <cctype>

using namespace adt;

namespace json
{

Token
Lexer::next()
{
    skipWhitespace();
    if (m_pos >= m_sJson.getSize()) return {};

    Token tok {};

    switch (m_sJson[m_pos])
    {
        case '{':
        tok = {
            .eType = TOKEN_TYPE::L_BRACE,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        case '}':
        tok = Token {
            .eType = TOKEN_TYPE::R_BRACE,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        case '"':
        tok = nextString();
        advance(1);
        break;

        case ':':
        tok = {
            .eType = TOKEN_TYPE::COLON,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        tok = nextNumber();
        break;

        case '[':
        tok = {
            .eType = TOKEN_TYPE::L_BRACKET,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        case ']':
        tok = {
            .eType = TOKEN_TYPE::R_BRACKET,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        case ',':
        tok = {
            .eType = TOKEN_TYPE::COMMA,
            .sLiteral = {&m_sJson[m_pos], 1},
            .raw = static_cast<decltype(Token::raw)>(m_row),
            .column = static_cast<decltype(Token::column)>(m_column),
        };
        advance(1);
        break;

        default:
        tok = nextStringNoQuotes();
        break;
    }

    return tok;
}

void
Lexer::skipWhitespace()
{
    while (m_pos < m_sJson.getSize() && isspace(m_sJson[m_pos]))
    {
        if (m_sJson[m_pos] == '\n')
        {
            ++m_row;
            m_column = 0;
        }

        advance(1);
    }
}

Token
Lexer::nextString()
{
    assert(m_sJson[m_pos] == '"');

    advance(1);
    if (done()) return {};

    auto fPos = m_pos;

    bool bEsc = false;
    for (; m_pos < m_sJson.getSize(); advance(1))
    {
        if (!bEsc && m_sJson[m_pos] == '\\')
        {
            bEsc = true;
            continue;
        }

        if (m_sJson[m_pos] == '"' && !bEsc)
            break;

        bEsc = false;
    }

    return {
        .eType = TOKEN_TYPE::QUOTED_STRING,
        .sLiteral = {&m_sJson[fPos], m_pos - fPos},
        .raw = static_cast<decltype(Token::raw)>(m_row),
        .column = static_cast<decltype(Token::column)>(m_column - (m_pos - fPos + 1)),
    };
}

Token
Lexer::nextStringNoQuotes()
{
    auto fPos = m_pos;

    while (m_pos < m_sJson.getSize() && isalnum(m_sJson[m_pos]))
        advance(1);

    return {
        .eType = TOKEN_TYPE::STRING,
        .sLiteral = {&m_sJson[fPos], m_pos - fPos},
        .raw = static_cast<decltype(Token::raw)>(m_row),
        .column = static_cast<decltype(Token::column)>(m_column - (m_pos - fPos)),
    };
}

Token
Lexer::nextNumber()
{
    assert(std::isdigit(m_sJson[m_pos]));

    auto fPos = m_pos;
    TOKEN_TYPE eType = TOKEN_TYPE::NUMBER;

    while (m_pos < m_sJson.getSize() && (isdigit(m_sJson[m_pos]) || m_sJson[m_pos] == '.'))
    {
        if (m_sJson[m_pos] == '.') 
            eType = TOKEN_TYPE::FLOAT;

        advance(1);
    }

    return {
        .eType = eType,
        .sLiteral = {&m_sJson[fPos], m_pos - fPos},
        .raw = static_cast<decltype(Token::raw)>(m_row),
        .column = static_cast<decltype(Token::column)>(m_column - (m_pos - fPos)),
    };
}

} /* namespace json */
