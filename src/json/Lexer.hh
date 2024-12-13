#pragma once

#include "adt/IAllocator.hh"
#include "adt/String.hh"

namespace json
{

enum RESULT : adt::u8 { OK, FAIL };

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
    adt::String sFile;
    adt::u32 pos = 0;

    Lexer() = default;
};

void LexerDestroy(Lexer* s, adt::IAllocator* pAlloc);
RESULT LexerLoadFile(Lexer* s, adt::IAllocator* pAlloc, adt::String path);
Token LexerNext(Lexer* s);

void LexerSkipWhiteSpace(Lexer* s);
Token LexerNumber(Lexer* s);
Token LexerStringNoQuotes(Lexer* s);
Token LexerString(Lexer* s);
Token LexerChar(Lexer* s, enum Token::TYPE type);

} /* namespace json */
