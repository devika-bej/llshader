#ifndef LLSHADER_AST_AST_H
#define LLSHADER_AST_AST_H

#include "llshader/Lexer/OperatorFilter.h"
#include "llshader/Lexer/Token.h"
#include <any>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

typedef llvm::StringMap<std::any> ProgramInfo;
class AST;
class Program;
class Statement;
class Expression;

class CompoundSt;
class Scoped;
class DefExpr;
class Declaration;
class Conditional;
class Loop;
class For;
class While;
class DoWhile;
class LoopMod;

class Literal;
class TypeConstructor;
class BinaryExpression;
class UnaryExpression;
class LValue;
class Assignment;
class VariableRef;
class IncDec;
class TypeCast;
class CompoundEx;

using StmtList = std::vector<Statement *>;
using ExprList = std::vector<Expression *>;
using DeclList = std::vector<Declaration *>;
using DefEList = std::vector<DefExpr *>;

class ASTVisitor
{
  public:
    virtual ~ASTVisitor() {};
    virtual void visit(Program &) {};
    virtual void visit(Statement &) {};
    virtual void visit(Expression &) {};

    virtual void visit(CompoundSt &) {};
    virtual void visit(Scoped &) {};
    virtual void visit(DefExpr &) {};
    virtual void visit(Declaration &) {};
    virtual void visit(Conditional &) {};
    virtual void visit(Loop &) {};
    virtual void visit(For &) {};
    virtual void visit(While &) {};
    virtual void visit(DoWhile &) {};
    virtual void visit(LoopMod &) {};

    virtual void visit(Literal &) {};
    virtual void visit(TypeConstructor &) {};
    virtual void visit(BinaryExpression &) {};
    virtual void visit(UnaryExpression &) {};
    virtual void visit(LValue &) {};
    virtual void visit(Assignment &) {};
    virtual void visit(VariableRef &) {};
    virtual void visit(IncDec &) {};
    virtual void visit(TypeCast &) {};
    virtual void visit(CompoundEx &) {};
};

// Base nodes

class AST
{
  public:
    virtual ~AST() {}
    virtual void accept(ASTVisitor &V) = 0;
};

class Program : public AST
{
    StmtList *SL;
    ProgramInfo Info;

  public:
    Program(StmtList *SL) : SL(SL) {};
    Program(StmtList *SL, ProgramInfo Info) : SL(SL), Info(Info) {};

    StmtList *getSL() const { return SL; };
    ProgramInfo getInfo() const { return Info; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Statement : public AST
{
  public:
    enum StmtKind
    {
        StmtComp,
        StmtScoped,
        StmtDecl,
        StmtCond,
        StmtLoop,
        StmtLoopMod,
    };

  private:
    const StmtKind Kind;

  public:
    Statement(StmtKind Kind) : Kind(Kind) {}

    StmtKind getKind() const { return Kind; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Expression : public AST
{
  public:
    enum ExprKind
    {
        ExprLit,
        ExprTypeCon,
        ExprBin,
        ExprUn,
        ExprAssmt,
        ExprRef,
        ExprIncDec,
        ExprTypeCast,
        ExprCompEx,
    };

  private:
    const ExprKind Kind;

  public:
    Expression(ExprKind Kind) : Kind(Kind) {}
    virtual TokenKind getType() const = 0;

    ExprKind getKind() const { return Kind; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

// Statement nodes

class CompoundSt : public Statement
{
    ExprList *EL;

  public:
    CompoundSt(ExprList *EL) : Statement(StmtComp), EL(EL) {};

    ~CompoundSt()
    {
        for (auto E : *EL)
        {
            delete E;
        }
        delete EL;
    };

    ExprList *getEL() const { return EL; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Scoped : public Statement
{
    StmtList *SL;

  public:
    Scoped(StmtList *SL) : Statement(StmtScoped), SL(SL) {};

    ~Scoped()
    {
        for (auto S : *SL)
        {
            delete S;
        }
        delete SL;
    };

    StmtList *getSL() const { return SL; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Statement *S)
    {
        return S->getKind() == StmtScoped;
    }
};

class DefExpr : public AST
{
    StringRef Id;
    Expression *Value = nullptr;

  public:
    DefExpr(StringRef Id) : Id(Id) {};
    DefExpr(StringRef Id, Expression *Value) : Id(Id), Value(Value) {};

    ~DefExpr()
    {
        if (Value != nullptr)
        {
            delete Value;
        }
    };

    StringRef getId() const { return Id; };
    Expression *getValue() const { return Value; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Declaration : public Statement
{
    TokenKind Type;
    DefEList *Defs;

  public:
    Declaration(TokenKind Type, DefEList *Defs)
        : Statement(StmtDecl), Type(Type), Defs(Defs) {};

    ~Declaration()
    {
        for (auto Def : *Defs)
        {
            delete Def;
        }
        delete Defs;
    };

    TokenKind getType() const { return Type; };
    DefEList *getDefs() const { return Defs; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Statement *S) { return S->getKind() == StmtDecl; }
};

class Conditional : public Statement
{
    Expression *Condition;
    Statement *ThenStmt;
    Statement *ElseStmt = nullptr;

  public:
    Conditional(Expression *Condition, Statement *ThenStmt,
                Statement *ElseStmt = nullptr)
        : Statement(StmtCond), Condition(Condition), ThenStmt(ThenStmt),
          ElseStmt(ElseStmt) {};

    ~Conditional()
    {
        delete Condition;
        delete ThenStmt;
        if (ElseStmt != nullptr)
            delete ElseStmt;
    };

    Expression *getCondition() const { return Condition; };
    Statement *getThen() const { return ThenStmt; };
    Statement *getElse() const { return ElseStmt; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Loop : public Statement
{
  public:
    enum LoopKind
    {
        LoopFor,
        LoopWhile,
        LoopDoWhile
    };

  private:
    LoopKind Kind;

  public:
    Loop(LoopKind Kind) : Statement(StmtLoop), Kind(Kind) {};

    LoopKind getKind() const { return Kind; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Statement *S) { return S->getKind() == StmtLoop; }
};

class For : public Loop
{
    Declaration *Init;
    Expression *Condition;
    CompoundEx *Update;
    Statement *Body;

  public:
    For(Statement *Body, Declaration *Init = nullptr,
        Expression *Condition = nullptr, CompoundEx *Update = nullptr)
        : Loop(LoopFor), Init(Init), Condition(Condition), Update(Update),
          Body(Body) {};

    ~For()
    {
        if (Init)
            delete Init;
        if (Condition)
            delete Condition;
        if (Update)
            delete Update;
        delete Body;
    };

    Declaration *getInit() const { return Init; };
    Expression *getCondition() const { return Condition; };
    CompoundEx *getUpdate() const { return Update; };
    Statement *getBody() const { return Body; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class While : public Loop
{
    Expression *Condition;
    Statement *Body;

  public:
    While(Expression *Condition, Statement *Body)
        : Loop(LoopWhile), Condition(Condition), Body(Body) {};

    ~While()
    {
        delete Condition;
        delete Body;
    };

    Expression *getCondition() const { return Condition; };
    Statement *getBody() const { return Body; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class DoWhile : public Loop
{
    Expression *Condition;
    Statement *Body;

  public:
    DoWhile(Expression *Condition, Statement *Body)
        : Loop(LoopDoWhile), Condition(Condition), Body(Body) {};

    ~DoWhile()
    {
        delete Condition;
        delete Body;
    };

    Expression *getCondition() const { return Condition; };
    Statement *getBody() const { return Body; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class LoopMod : public Statement
{
    TokenKind Mod;

  public:
    LoopMod(TokenKind Mod) : Statement(StmtLoopMod), Mod(Mod) {};

    ~LoopMod() {};

    TokenKind getMod() const { return Mod; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }

    static bool classof(const Statement *S)
    {
        return S->getKind() == StmtLoopMod;
    }
};

// Expression nodes

class Literal : public Expression
{
  public:
    enum LitKind
    {
        Integer,
        FloatingPoint,
        String
    };

  private:
    LitKind Kind;
    StringRef Value;

  public:
    Literal(LitKind Kind, StringRef Value)
        : Expression(ExprLit), Kind(Kind), Value(Value) {};

    LitKind getKind() const { return Kind; };
    StringRef getValue() const { return Value; };
    TokenKind getType() const override
    {
        if (Kind == Integer)
            return TokenKind::kw_int;
        if (Kind == FloatingPoint)
            return TokenKind::kw_float;
        return TokenKind::kw_string;
    }

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E) { return E->getKind() == ExprLit; }
};

class TypeConstructor : public Expression
{
    TokenKind Type;
    ExprList *Values;

  public:
    TypeConstructor(TokenKind Type, ExprList *Values)
        : Expression(ExprTypeCon), Type(Type), Values(Values) {};

    ~TypeConstructor()
    {
        for (auto Val : *Values)
        {
            delete Val;
        }
        delete Values;
    };

    TokenKind getType() const override { return Type; };
    ExprList *getValues() const { return Values; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E)
    {
        return E->getKind() == ExprTypeCon;
    }
};

class BinaryExpression : public Expression
{
    StringRef Op;
    Expression *E1;
    Expression *E2;

  public:
    BinaryExpression(StringRef Op, Expression *E1, Expression *E2)
        : Expression(ExprBin), Op(Op), E1(E1), E2(E2) {};

    ~BinaryExpression()
    {
        delete E1;
        delete E2;
    }

    StringRef getOp() const { return Op; };
    Expression *getE1() const { return E1; };
    Expression *getE2() const { return E2; };
    TokenKind getType() const override
    {
        OperatorFilter opFilter;
        TokenKind type1 = E1->getType();
        TokenKind type2 = E2->getType();

        if (type1 == TokenKind::kw_string || type2 == TokenKind::kw_string)
        {
            if (type1 != type2)
                return TokenKind::kw_err;
            if (Op != "==" && Op != "!=")
                return TokenKind::kw_err;
            return TokenKind::kw_int;
        }

        if (opFilter.getTokenKind(Op.str()) == TokenKind::log_op ||
            opFilter.getTokenKind(Op.str()) == TokenKind::comp_op)
            return TokenKind::kw_int;

        auto isComplex = [](TokenKind type) -> bool
        {
            return (
                type == TokenKind::kw_color || type == TokenKind::kw_normal ||
                type == TokenKind::kw_point || type == TokenKind::kw_vector ||
                type == TokenKind::kw_matrix);
        };

        if (isComplex(type1) && isComplex(type2))
        {
            if (type1 != type2)
                return TokenKind::kw_err;
            return type1;
        }

        if (isComplex(type1) || isComplex(type2))
        {
            auto nonComplex = isComplex(type1) ? type2 : type1;
            auto complex = isComplex(type1) ? type1 : type2;
            if (nonComplex == TokenKind::kw_int ||
                nonComplex == TokenKind::kw_float)
                return complex;
            return TokenKind::kw_err;
        }

        if (type1 == type2)
            return type1;

        return TokenKind::kw_float;
    };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E) { return E->getKind() == ExprBin; }
};

class UnaryExpression : public Expression
{
    StringRef Op;
    Expression *E;

  public:
    UnaryExpression(StringRef Op, Expression *E)
        : Expression(ExprUn), Op(Op), E(E) {};

    ~UnaryExpression() { delete E; };

    StringRef getOp() const { return Op; };
    Expression *getE() const { return E; };
    TokenKind getType() const override { return TokenKind::kw_int; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E) { return E->getKind() == ExprUn; }
};

class LValue : public AST
{
    StringRef Id;
    ExprList *Indices = nullptr;

  public:
    LValue(StringRef Id, ExprList *Indices = nullptr)
        : Id(Id), Indices(Indices) {};

    ~LValue()
    {
        if (Indices != nullptr)
        {
            for (auto E : *Indices)
            {
                delete E;
            }
            delete Indices;
        }
    };

    StringRef getId() const { return Id; };
    ExprList *getIndices() const { return Indices; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Assignment : public Expression
{
    LValue *Id;
    StringRef Op;
    Expression *Value;

  public:
    Assignment(LValue *Id, StringRef Op, Expression *Value)
        : Expression(ExprAssmt), Id(Id), Op(Op), Value(Value) {};

    ~Assignment()
    {
        delete Id;
        delete Value;
    };

    LValue *getId() const { return Id; };
    Expression *getValue() const { return Value; };
    StringRef getOp() const { return Op; };
    TokenKind getType() const override
    {
        if (Op == "=")
            return Value->getType();
        // get type for the lvalue
    };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Statement *S)
    {
        return S->getKind() == ExprAssmt;
    }
};

class VariableRef : public Expression
{
    StringRef Id;
    Expression *Deref;
    TokenKind Type = TokenKind::kw_void;

  public:
    VariableRef(StringRef Id, Expression *Deref)
        : Expression(ExprRef), Id(Id), Deref(Deref) {};

    ~VariableRef() { delete Deref; };

    StringRef getId() const { return Id; };
    Expression *getDeref() const { return Deref; };
    TokenKind getType() const override { return Type; };
    void setType(TokenKind newType) { Type = newType; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E) { return E->getKind() == ExprRef; }
};

class IncDec : public Expression
{
    StringRef Op;
    VariableRef *Id;

  public:
    IncDec(StringRef Op, VariableRef *Id)
        : Expression(ExprIncDec), Op(Op), Id(Id) {};

    ~IncDec() { delete Id; };

    StringRef getOp() const { return Op; };
    VariableRef *getId() const { return Id; };
    TokenKind getType() const override { return Id->getType(); };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E)
    {
        return E->getKind() == ExprIncDec;
    }
};

class TypeCast : public Expression
{
    TokenKind Type;
    Expression *E;

  public:
    TypeCast(TokenKind Type, Expression *E)
        : Expression(ExprTypeCast), Type(Type), E(E) {};

    ~TypeCast() { delete E; };

    TokenKind getType() const override { return Type; };
    Expression *getE() const { return E; };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E)
    {
        return E->getKind() == ExprTypeCast;
    }
};

class CompoundEx : public Expression
{
    ExprList *EL;

  public:
    CompoundEx(ExprList *EL) : Expression(ExprCompEx), EL(EL) {};

    ~CompoundEx()
    {
        for (auto E : *EL)
        {
            delete E;
        }
        delete EL;
    };

    ExprList *getEL() const { return EL; };
    TokenKind getType() const override
    {
        if (EL->size() == 1)
            return (*EL)[0]->getType();
        return TokenKind::kw_void;
    };

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
    static bool classof(const Expression *E)
    {
        return E->getKind() == ExprCompEx;
    }
};

#endif