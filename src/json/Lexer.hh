#pragma once

#include "adt/String.hh"
#include "adt/Allocator.hh"

namespace json
{

struct Token
{
    enum TYPE : adt::u8
    {
        LBRACE = '{',
        RBRACE = '}',
        LBRACKET = '[',
        RBRACKET = ']',
        QUOTE = '"',
        IDENT = 'I',
        NUMBER = 'N',
        TRUE_ = 'T',
        FALSE_ = 'F',
        NULL_ = 'n',
        ASSIGN = ':',
        COMMA = ',',
        DOT = '.',
        UNHANDLED = 'X',
        EOF_ = '\0',
    } type;
    adt::String sLiteral;
};

struct Lexer
{
    adt::Allocator* pAlloc {};
    adt::String sFile;
    adt::u32 pos = 0;

    Lexer() = default;
    Lexer(adt::Allocator* p) : pAlloc(p) {}
};

void LexerDestroy(Lexer* s);
void LexerLoadFile(Lexer* s, adt::String path);
Token LexerNext(Lexer* s);

void LexerSkipWhiteSpace(Lexer* s);
Token LexerNumber(Lexer* s);
Token LexerStringNoQuotes(Lexer* s);
Token LexerString(Lexer* s);
Token LexerChar(Lexer* s, enum Token::TYPE type);

} /* namespace json */
