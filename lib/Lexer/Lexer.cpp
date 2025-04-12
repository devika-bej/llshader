#include "llshader/Lexer/Lexer.h"
#include <iostream>

namespace charinfo
{
LLVM_READNONE inline static bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r' ||
           c == '\n';
}
LLVM_READNONE inline static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}
LLVM_READNONE inline static bool isHexDigit(char c)
{
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
LLVM_READNONE inline static bool isSign(char c)
{
    return (c == '-') || (c == '+');
}
LLVM_READNONE inline static bool isLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
LLVM_READNONE inline static bool isLetter_(char c)
{
    return isLetter(c) || c == '_';
}
LLVM_READNONE inline static bool isLetterDigit_(char c)
{
    return isLetter_(c) || isDigit(c);
}
} // namespace charinfo

void Lexer::next(Token &token)
{
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
        ++BufferPtr;

    if (!*BufferPtr)
    {
        token.Kind = TokenKind::eof;
        return;
    }

    // operator/punctuator
    if (!charinfo::isWhitespace(*BufferPtr) &&
        !charinfo::isLetterDigit_(*BufferPtr) && *BufferPtr != '"')
    {
        const char *End = BufferPtr + 1;
        while (*End && !charinfo::isLetterDigit_(*End) &&
               !charinfo::isWhitespace(*End))
            ++End;
        std::string chars = StringRef(BufferPtr, End - BufferPtr).str();
        TokenKind kind = opFilter.getTokenKind(chars);
        if (kind != TokenKind::unknown)
        {
            formToken(token, End, kind);
            return;
        }
        else
        {
            while (BufferPtr != End)
            {
                std::string curChar = StringRef(BufferPtr, 1).str();
                TokenKind curKind = opFilter.getTokenKind(curChar);
                formToken(token, BufferPtr + 1, curKind);
                return;
            }
        }
    }

    // floating point
    if ((charinfo::isDigit(*BufferPtr) &&
         StringRef(BufferPtr, 2).str() != "0x") ||
        *BufferPtr == '.')
    {
        auto Start = BufferPtr;
        if (charinfo::isDigit(*BufferPtr))
        {
            const char *End = getDigitSequence();
            if (*End == '.')
            {
                BufferPtr = End;
                End = getDecimalPart();
                if (*End == 'e' || *End == 'E')
                {
                    BufferPtr = End;
                    End = getExponent();
                }
            }
            else if (*End == 'e' || *End == 'E')
            {
                BufferPtr = End;
                End = getExponent();
            }
            else
            {
                BufferPtr = Start;
                formToken(token, End, TokenKind::integer);
                return;
            }

            BufferPtr = Start;
            formToken(token, End, TokenKind::floating_point);
            return;
        }
        else if (*BufferPtr == '.')
        {
            const char *End = getDecimalPart();
            if (*End == 'e' || *End == 'E')
            {
                BufferPtr = End;
                End = getExponent();
            }

            BufferPtr = Start;
            formToken(token, End, TokenKind::floating_point);
            return;
        }
    }

    // integer
    if (charinfo::isSign(*BufferPtr) || charinfo::isDigit(*BufferPtr) ||
        StringRef(BufferPtr, 2).str() == "0x")
    {
        auto Start = BufferPtr;
        if (charinfo::isSign(*BufferPtr))
        {
            BufferPtr += 1;
        }
        if (StringRef(BufferPtr, 2).str() == "0x")
        {
            const char *End = getHexDigitSequence();
            BufferPtr = Start;
            formToken(token, End, TokenKind::integer);
            return;
        }
        else if (charinfo::isDigit(*BufferPtr))
        {
            const char *End = getDigitSequence();
            BufferPtr = Start;
            formToken(token, End, TokenKind::integer);
            return;
        }
    }

    // string
    if (*BufferPtr == '"')
    {
        BufferPtr += 1;
        const char *End = BufferPtr + 1;
        while (End && *End != '"')
            ++End;
        formToken(token, End, TokenKind::string_literal);
        BufferPtr += 1;
        return;
    }

    // identifier/keyword
    if (charinfo::isLetter_(*BufferPtr))
    {
        const char *End = BufferPtr + 1;
        while (charinfo::isLetterDigit_(*End))
            ++End;
        StringRef text(BufferPtr, End - BufferPtr);
        TokenKind kind = kwFilter.lookup(text);
        formToken(token, End, kind);
        return;
    }
}

const char *Lexer::getDigitSequence()
{
    const char *End = BufferPtr + 1;
    while (End && charinfo::isDigit(*End))
        ++End;
    return End;
}

const char *Lexer::getHexDigitSequence()
{
    const char *End = BufferPtr + 2;
    while (End && charinfo::isHexDigit(*End))
        ++End;
    return End;
}

const char *Lexer::getDecimalPart()
{
    const char *End = BufferPtr;
    if (*End == '.')
    {
        ++End;
        while (*End && charinfo::isDigit(*End))
            ++End;
    }
    return End;
}

const char *Lexer::getExponent()
{
    const char *End = BufferPtr;
    if (*End == 'e' || *End == 'E')
    {
        ++End;
        if (*End && charinfo::isSign(*End))
            ++End;
        if (*End && charinfo::isDigit(*End))
        {
            while (*End && charinfo::isDigit(*End))
                ++End;
        }
    }
    return End;
}

void Lexer::formToken(Token &Tok, const char *TokEnd, TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}