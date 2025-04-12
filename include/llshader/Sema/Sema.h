#ifndef LLSHADER_SEMA_SEMA_H
#define LLSHADER_SEMA_SEMA_H

#include "llshader/AST/AST.h"
#include "llshader/Lexer/Lexer.h"

class Sema {
public:
  bool semantic(AST *Tree);
};

#endif
