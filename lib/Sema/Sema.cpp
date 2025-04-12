#include "llshader/Sema/Sema.h"
#include "llshader/Sema/SymbolTable.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace
{
class ProgramCheck : public ASTVisitor
{
    std::shared_ptr<SymbolTable> curSymTab;
    int loopLevel = 0;
    bool hasError = false;

  public:
    ProgramCheck()
        : curSymTab(std::make_shared<SymbolTable>()), loopLevel(0),
          hasError(false) {};
    bool hasErrorFunc() { return hasError; }

    void visit(Program &Node) override
    {
        if (Node.getSL())
        {
            for (auto S : *Node.getSL())
            {
                S->accept(*this);
            }
        }
    }

    // Statement checks

    void visit(Statement &Node) override
    {
        switch (Node.getKind())
        {
        case Statement::StmtComp:
            Node.accept(*this);
            break;
        case Statement::StmtScoped:
            Node.accept(*this);
            break;
        case Statement::StmtDecl:
            Node.accept(*this);
            break;
        case Statement::StmtCond:
            Node.accept(*this);
            break;
        case Statement::StmtLoop:
            loopLevel++;
            Node.accept(*this);
            loopLevel--;
            break;
        case Statement::StmtLoopMod:
            Node.accept(*this);
            break;
        default:
            llvm_unreachable("Unknown statement kind");
        }
    }

    void visit(CompoundSt &Node) override
    {
        if (Node.getEL())
        {
            for (auto E : *Node.getEL())
            {
                E->accept(*this);
            }
        }
    }

    void visit(Scoped &Node) override
    {
        auto parSymTab = curSymTab;
        curSymTab = std::make_shared<SymbolTable>(parSymTab);
        if (Node.getSL())
        {
            for (auto S : *Node.getSL())
            {
                S->accept(*this);
            }
        }
        curSymTab = parSymTab;
    }

    void visit(Declaration &Node) override
    {
        TokenKind type = Node.getType();
        if (Node.getDefs())
        {
            for (auto &def : *Node.getDefs())
            {
                auto id = def->getId().str();
                auto exists = curSymTab->lookup(id, false);
                if (exists != TokenKind::kw_void)
                {
                    llvm::errs()
                        << "Error: Redeclaration of variable " << id << "\n";
                    hasError = true;
                    return;
                }
                if (def->getValue())
                {
                    def->getValue()->accept(*this);
                    TokenKind rhsType = def->getValue()->getType();
                    if (type != rhsType && !(type == TokenKind::kw_float &&
                                             rhsType == TokenKind::kw_int))
                    {
                        llvm::errs() << "Error: Type mismatch in declaration "
                                        "of variable "
                                     << id << "\n";
                        hasError = true;
                        return;
                    }
                }
                curSymTab->insert(id, type);
            }
        }
    }

    void visit(Conditional &Node) override
    {
        Node.getCondition()->accept(*this);
        if (Node.getCondition()->getType() != TokenKind::kw_int)
        {
            llvm::errs() << "Error: Condition must be an integer\n";
            hasError = true;
            return;
        }
        Node.getThen()->accept(*this);
        if (Node.getElse())
            Node.getElse()->accept(*this);
    }

    void visit(Loop &Node) override
    {
        switch (Node.getKind())
        {
        case Loop::LoopFor:
            Node.accept(*this);
            break;
        case Loop::LoopWhile:
            Node.accept(*this);
            break;
        case Loop::LoopDoWhile:
            Node.accept(*this);
            break;
        default:
            llvm_unreachable("Unknown loop kind");
        }
    }

    void visit(For &Node) override
    {
        auto parSymTab = curSymTab;
        if (Node.getInit())
        {
            curSymTab = std::make_shared<SymbolTable>(parSymTab);
            Node.getInit()->accept(*this);
        }
        if (Node.getCondition())
        {
            Node.getCondition()->accept(*this);
            if (Node.getCondition()->getType() != TokenKind::kw_int)
            {
                llvm::errs() << "Error: Condition must be an integer\n";
                hasError = true;
                return;
            }
        }
        if (Node.getUpdate())
            Node.getUpdate()->accept(*this);
        Node.getBody()->accept(*this);
        curSymTab = parSymTab;
    }
};
} // namespace

bool Sema::semantic(AST *Tree)
{
    if (!Tree)
        return false;
    ProgramCheck Check;
    Tree->accept(Check);
    return !Check.hasErrorFunc();
}