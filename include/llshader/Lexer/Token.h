#ifndef LLSHADER_LEXER_TOKENKIND_H
#define LLSHADER_LEXER_TOKENKIND_H

#include "llshader/Basic/TokenKinds.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SMLoc.h"
#include <unordered_set>

using llvm::SMLoc;
using llvm::StringRef;

using namespace llshader;
using tok::TokenKind;

class Token {
  friend class Lexer;

private:
  TokenKind Kind;
  StringRef Text;

public:
  TokenKind getKind() const { return Kind; }
  StringRef getText() const { return Text; }
  size_t getLength() const { return Text.size(); }

  bool is(TokenKind K) const { return Kind == K; }
  bool is(std::unordered_set<tok::TokenKind> K) const { return K.count(Kind); }
  template <typename... Ts> bool isOneOf(Ts... Ks) const {
    return (... || is(Ks));
  }

  SMLoc getLocation() const { return SMLoc::getFromPointer(Text.data()); }
};
#endif
