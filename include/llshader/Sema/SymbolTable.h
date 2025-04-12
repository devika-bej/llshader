#include "llshader/AST/AST.h"
#include "llshader/Lexer/Token.h"
#include "llshader/Parser/Parser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <iostream>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <unordered_map>
#include <vector>

using namespace llshader;
using tok::TokenKind;

class SymbolTable
{
  public:
    SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr)
        : parent(parent) {};

    void insert(const std::string &name, TokenKind type) { table[name] = type; }

    TokenKind lookup(const std::string &name, bool recursive = true)
    {
        if (table.find(name) != table.end())
            return table[name];
        if (parent && recursive)
            return parent->lookup(name);
        return TokenKind::kw_void;
    }

  private:
    std::unordered_map<std::string, TokenKind> table;
    std::shared_ptr<SymbolTable> parent;
};
