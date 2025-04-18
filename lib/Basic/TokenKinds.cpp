#include "llshader/Basic/TokenKinds.h"

using namespace llshader;

static const char *const TokNames[] = {
#define TOK(ID) #ID,
#define KEYWORD(ID, FLAG) #ID,
#include "llshader/Basic/TokenKinds.def"
    nullptr};

const char *tok::getTokenName(TokenKind Kind) { return TokNames[Kind]; }

const char *tok::getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {

#define PUNCTUATOR(ID, SP)                                                     \
  case ID:                                                                     \
    return SP;
#include "llshader/Basic/TokenKinds.def"

  default:
    break;
  }
  return nullptr;
}

const char *tok::getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {

#define KEYWORD(ID, FLAG)                                                      \
  case kw_##ID:                                                                \
    return #ID;
#include "llshader/Basic/TokenKinds.def"

  default:
    break;
  }
  return nullptr;
}
