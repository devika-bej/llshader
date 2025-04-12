#ifndef LLSHADER_LEXER_LEXER_H
#define LLSHADER_LEXER_LEXER_H

#include "llshader/Basic/Diagnostic.h"
#include "llshader/Lexer/KeywordFilter.h"
#include "llshader/Lexer/OperatorFilter.h"
#include "llshader/Lexer/Token.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"

using llvm::StringRef;
using namespace llshader;

class Lexer
{
    unsigned CurrBuffer = 0;
    StringRef Buffer;
    const char *BufferPtr;

    llvm::SourceMgr &SrcMgr;

    KeywordFilter kwFilter;
    OperatorFilter opFilter;

    DiagnosticsEngine Diags;

  public:
    Lexer(llvm::SourceMgr &SrcMgr, DiagnosticsEngine &Diags)
        : SrcMgr(SrcMgr), Diags(Diags)
    {
        CurrBuffer = SrcMgr.getMainFileID();
        Buffer = SrcMgr.getMemoryBuffer(CurrBuffer)->getBuffer();
        BufferPtr = Buffer.begin();
    }

    DiagnosticsEngine &getDiagnostics() { return Diags; }

    void next(Token &Tok);

    // Might be useful
    Token peek(unsigned N = 0);

  private:
    const char* getDigitSequence();
    const char* getHexDigitSequence();
    const char* getDecimalPart();
    const char* getExponent();
    void formToken(Token &Result, const char *TokEnd, TokenKind Kind);

    SMLoc getLoc() { return SMLoc::getFromPointer(BufferPtr); }
};

#endif
