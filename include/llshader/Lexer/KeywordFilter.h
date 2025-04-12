#ifndef LLSHADER_LEXER_KEYWORDFILTER_H
#define LLSHADER_LEXER_KEYWORDFILTER_H

#include "llshader/Lexer/Token.h"
#include "llvm/ADT/StringRef.h"
#include <iostream>
#include <unordered_map>

using llvm::StringRef;
using namespace llshader;

class KeywordFilter
{
  private:
    std::unordered_map<std::string, TokenKind> KeywordMap;

  public:
    KeywordFilter()
    {
        KeywordMap = {{"int", TokenKind::kw_int},
                      {"float", TokenKind::kw_float},
                      {"point", TokenKind::kw_point},
                      {"vector", TokenKind::kw_vector},
                      {"normal", TokenKind::kw_normal},
                      {"color", TokenKind::kw_color},
                      {"matrix", TokenKind::kw_matrix},
                      {"string", TokenKind::kw_string},
                      {"void", TokenKind::kw_void},
                      {"if", TokenKind::kw_if},
                      {"else", TokenKind::kw_else},
                      {"while", TokenKind::kw_while},
                      {"for", TokenKind::kw_for},
                      {"do", TokenKind::kw_do},
                      {"break", TokenKind::kw_break},
                      {"continue", TokenKind::kw_continue}};
    }

    TokenKind lookup(StringRef Identifier) const
    {
        auto it = KeywordMap.find(Identifier.str());
        return (it != KeywordMap.end()) ? it->second : TokenKind::identifier;
    }

    bool isSimpleType(StringRef Identifier) const
    {
        auto it = KeywordMap.find(Identifier.str());
        if (it != KeywordMap.end())
        {
            auto type = it->second;
            if (type == TokenKind::kw_int || type == TokenKind::kw_float ||
                type == TokenKind::kw_string)
                return true;
        }
        return false;
    }

    bool isComplexType(StringRef Identifier) const
    {
        auto it = KeywordMap.find(Identifier.str());
        if (it != KeywordMap.end())
        {
            auto type = it->second;
            if (type == TokenKind::kw_point || type == TokenKind::kw_vector ||
                type == TokenKind::kw_normal || type == TokenKind::kw_color ||
                type == TokenKind::kw_matrix)
                return true;
        }
        return false;
    }

    bool isType(StringRef Identifier) const
    {
        return isSimpleType(Identifier) || isComplexType(Identifier);
    }
};

#endif
