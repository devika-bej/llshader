#ifndef LLSHADER_BASIC_TOKENKINDS_H
#define LLSHADER_BASIC_TOKENKINDS_H

namespace llshader {
namespace tok {
enum TokenKind : unsigned short {
#define TOK(ID) ID,
#include "TokenKinds.def"
  NUM_TOKENS
};

const char *getTokenName(TokenKind Kind);
const char *getPunctuatorSpelling(TokenKind Kind);
const char *getKeywordSpelling(TokenKind Kind);

} // namespace tok
} // namespace llshader

#endif
