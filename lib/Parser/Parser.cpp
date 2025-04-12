#include "llshader/Parser/Parser.h"
#include "llshader/AST/AST.h"
#include "llshader/Lexer/Token.h"
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llshader;
using tok::TokenKind;

AST *Parser::parse()
{
    StmtList *SL = new StmtList();
    while (!Tok.is(TokenKind::eof))
    {
        Statement *curStmt = parseStmt();
        if (curStmt == nullptr)
            return nullptr;
        SL->push_back(curStmt);
    }
    Program *P = new Program(SL);
    AST *Res = llvm::dyn_cast<AST>(P);
    expect(TokenKind::eof);
    return Res;
}

Statement *Parser::parseStmt()
{
    auto ErrorHandler = [this]()
    {
        Diags.report(Tok.getLocation(), diag::err_unexpected_token,
                     Tok.getText());
        return nullptr;
    };

    // Scoped statement
    if (Tok.getText() == "{")
    {
        advance();
        StmtList *curScoped = new StmtList();
        while (Tok.getText() != "}")
        {
            Statement *curStmt = parseStmt();
            curScoped->push_back(curStmt);
            if (curStmt == nullptr)
                return ErrorHandler();
        }
        advance();
        return new Scoped(curScoped);
    }

    KeywordFilter KWFilter;

    // Conditional statement
    if (Tok.getKind() == TokenKind::kw_if)
    {
        advance();
        if (Tok.getText() != "(")
            return ErrorHandler();
        advance();
        Expression *condition = parseExpr();
        if (condition == nullptr)
            return ErrorHandler();
        while (Tok.getText() != ")")
        {
            if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
            {
                StringRef op = Tok.getText();
                advance();
                Expression *nextCondition = parseTerm();
                if (nextCondition == nullptr)
                    return ErrorHandler();
                condition = new BinaryExpression(op, condition, nextCondition);
            }
            else
                return ErrorHandler();
        }
        advance();
        Statement *thenStmt = parseStmt();
        if (thenStmt == nullptr)
            return ErrorHandler();
        if (Tok.getKind() == TokenKind::kw_else)
        {
            advance();
            Statement *elseStmt = parseStmt();
            if (elseStmt == nullptr)
                return ErrorHandler();
            return new Conditional(condition, thenStmt, elseStmt);
        }
        return new Conditional(condition, thenStmt);
    }

    // While statement
    if (Tok.getKind() == TokenKind::kw_while)
    {
        advance();
        if (Tok.getText() != "(")
            return ErrorHandler();
        advance();
        Expression *condition = parseExpr();
        if (condition == nullptr)
            return ErrorHandler();
        while (Tok.getText() != ")")
        {
            if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
            {
                StringRef op = Tok.getText();
                advance();
                Expression *nextCondition = parseTerm();
                if (nextCondition == nullptr)
                    return ErrorHandler();
                condition = new BinaryExpression(op, condition, nextCondition);
            }
            else
                return ErrorHandler();
        }
        advance();
        Statement *bodyStmt = parseStmt();
        if (bodyStmt == nullptr)
            return ErrorHandler();
        return new While(condition, bodyStmt);
    }

    // Do While statement
    if (Tok.getKind() == TokenKind::kw_do)
    {
        advance();
        Statement *bodyStmt = parseStmt();
        if (bodyStmt == nullptr)
            return ErrorHandler();
        if (Tok.getKind() != TokenKind::kw_while)
            return ErrorHandler();
        advance();
        if (Tok.getText() != "(")
            return ErrorHandler();
        advance();
        Expression *condition = parseExpr();
        if (condition == nullptr)
            return ErrorHandler();
        while (Tok.getText() != ")")
        {
            if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
            {
                StringRef op = Tok.getText();
                advance();
                Expression *nextCondition = parseTerm();
                if (nextCondition == nullptr)
                    return ErrorHandler();
                condition = new BinaryExpression(op, condition, nextCondition);
            }
            else
                return ErrorHandler();
        }
        advance();
        if (Tok.getText() != ";")
            return ErrorHandler();
        advance();
        return new DoWhile(condition, bodyStmt);
    }

    // For statement
    if (Tok.getKind() == TokenKind::kw_for)
    {
        advance();
        if (Tok.getText() != "(")
            return ErrorHandler();
        advance();
        Declaration *init = nullptr;
        if (Tok.getText() != ";")
        {
            init = llvm::dyn_cast<Declaration>(parseStmt());
            if (init == nullptr)
                return ErrorHandler();
        }
        else
        {
            advance();
        }

        Expression *condition = nullptr;
        if (Tok.getText() != ";")
        {
            condition = parseExpr();
            if (condition == nullptr)
                return ErrorHandler();
            while (Tok.getText() != ";")
            {
                if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                    Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                {
                    StringRef op = Tok.getText();
                    advance();
                    Expression *nextCondition = parseTerm();
                    if (nextCondition == nullptr)
                        return ErrorHandler();
                    condition =
                        new BinaryExpression(op, condition, nextCondition);
                }
                else
                    return ErrorHandler();
            }
        }
        advance();

        CompoundEx *update = nullptr;
        if (Tok.getText() != ")")
        {
            update = llvm::dyn_cast<CompoundEx>(parseExpr());
            if (update == nullptr)
                return ErrorHandler();
        }
        else
        {
            advance();
        }

        Statement *body = parseStmt();
        if (body == nullptr)
            return ErrorHandler();

        return new For(body, init, condition, update);
    }

    // Loop Mod statement
    if (Tok.getKind() == TokenKind::kw_break ||
        Tok.getKind() == TokenKind::kw_continue)
    {
        auto ret = new LoopMod(Tok.getKind());
        advance();
        if (Tok.getText() != ";")
            return ErrorHandler();
        advance();
        return ret;
    }

    // Declaration statement
    if (KWFilter.isType(Tok.getText()))
    {
        TokenKind type = KWFilter.lookup(Tok.getText());
        DefEList *defs = new DefEList();
        advance();
        while (Tok.getText() != ";")
        {
            if (!expect(TokenKind::identifier))
                return ErrorHandler();
            StringRef id = Tok.getText();
            advance();
            if (Tok.getText() == "=")
            {
                advance();

                Expression *value = parseExpr();
                if (value == nullptr)
                    return ErrorHandler();
                while (Tok.getText() != "," && Tok.getText() != ";")
                {
                    if (Tok.is(TokenKind::bin_op) ||
                        Tok.is(TokenKind::comp_op) ||
                        Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                    {
                        StringRef op = Tok.getText();
                        advance();
                        Expression *nextValue = parseTerm();
                        if (nextValue == nullptr)
                            return ErrorHandler();
                        value = new BinaryExpression(op, value, nextValue);
                    }
                    else
                        return ErrorHandler();
                }
                defs->push_back(new DefExpr(id, value));
                if (Tok.getText() == ",")
                    advance();
            }
            else if (Tok.getText() == ",")
            {
                defs->push_back(new DefExpr(id));
                advance();
            }
            else if (Tok.getText() == ";")
            {
                defs->push_back(new DefExpr(id));
            }
        }
        advance();
        return new Declaration(type, defs);
    }

    // Compound expressions statement
    if (Tok.getText() == ";")
    {
        return new CompoundSt(nullptr);
        advance();
    }
    else
    {
        CompoundEx *compEx = llvm::dyn_cast<CompoundEx>(parseExpr());
        if (Tok.getText() != ";")
            return ErrorHandler();
        ExprList *EL = compEx->getEL();
        return new CompoundSt(EL);
    }

    return ErrorHandler();
}

Expression *Parser::parseExpr()
{
    auto ErrorHandler = [this]()
    {
        Diags.report(Tok.getLocation(), diag::err_unexpected_token,
                     Tok.getText());
        return nullptr;
    };

    // Literal expression
    if (Tok.is(TokenKind::integer) || Tok.is(TokenKind::string_literal) ||
        Tok.is(TokenKind::floating_point))
    {
        Literal::LitKind kind =
            Tok.is(TokenKind::integer)
                ? Literal::Integer
                : (Tok.is(TokenKind::floating_point) ? Literal::FloatingPoint
                                                     : Literal::String);
        Literal *Lit = new Literal(kind, Tok.getText());
        advance();
        return Lit;
    }

    KeywordFilter KWFilter;

    // Type Constructor expression
    if (KWFilter.isType(Tok.getText()))
    {
        TokenKind type = KWFilter.lookup(Tok.getText());
        advance();
        if (Tok.getText() == "(")
        {
            advance();
            ExprList *values = new ExprList();
            while (Tok.getText() != ")")
            {
                Expression *value = parseExpr();
                if (value == nullptr)
                    return ErrorHandler();
                while (Tok.getText() != "," && Tok.getText() != ")")
                {
                    if (Tok.is(TokenKind::bin_op) ||
                        Tok.is(TokenKind::comp_op) ||
                        Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                    {
                        StringRef op = Tok.getText();
                        advance();
                        Expression *nextValue = parseTerm();
                        if (nextValue == nullptr)
                            return ErrorHandler();
                        value = new BinaryExpression(op, value, nextValue);
                    }
                    else
                        return ErrorHandler();
                }
                values->push_back(value);
                if (Tok.getText() == ",")
                    advance();
            }
            advance();
            return new TypeConstructor(type, values);
        }
        else
        {
            return ErrorHandler();
        }
    }

    // Unary expression
    if (Tok.is(TokenKind::un_op))
    {
        StringRef op = Tok.getText();
        advance();
        Expression *E = parseExpr();
        if (E == nullptr)
            return ErrorHandler();
        return new UnaryExpression(op, E);
    }

    // IncDec expression
    if (Tok.is(TokenKind::incdec_op))
    {
        StringRef op = Tok.getText();
        advance();
        VariableRef *Id = llvm::dyn_cast<VariableRef>(parseExpr());
        if (Id == nullptr)
            return ErrorHandler();
        return new IncDec(op, Id);
    }

    if (Tok.getText() == "(")
    {
        advance();

        // TypeCast expression
        if (KWFilter.isType(Tok.getText()))
        {
            TokenKind type = KWFilter.lookup(Tok.getText());
            advance();
            if (Tok.getText() != ")")
                return ErrorHandler();
            advance();
            Expression *E = parseTerm();
            if (E == nullptr)
                return ErrorHandler();
            return new TypeCast(type, E);
        }

        // Compound expression
        ExprList *EL = new ExprList();
        while (Tok.getText() != ")")
        {
            Expression *E = parseExpr();
            if (E == nullptr)
                return ErrorHandler();
            while (Tok.getText() != "," && Tok.getText() != ")")
            {
                if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                    Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                {
                    StringRef op = Tok.getText();
                    advance();
                    Expression *nextValue = parseTerm();
                    if (nextValue == nullptr)
                        return ErrorHandler();
                    E = new BinaryExpression(op, E, nextValue);
                }
                else
                    return ErrorHandler();
            }
            EL->push_back(E);
            if (Tok.getText() == ",")
                advance();
        }
        advance();
        return new CompoundEx(EL);
    }

    // Variable ref or assignment expression
    if (Tok.is(TokenKind::identifier))
    {
        StringRef id = Tok.getText();
        advance();
        if (Tok.getText() == "[")
        {
            advance();

            Expression *index = parseExpr();
            if (index == nullptr)
                return ErrorHandler();
            while (Tok.getText() != "]")
            {
                if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                    Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                {
                    StringRef op = Tok.getText();
                    advance();
                    Expression *nextIndex = parseTerm();
                    if (nextIndex == nullptr)
                        return ErrorHandler();
                    index = new BinaryExpression(op, index, nextIndex);
                }
                else
                    return ErrorHandler();
            }

            advance();
            if (Tok.getText() != "[" && Tok.getKind() != TokenKind::assignment)
            {
                return new VariableRef(id, index);
            }
            else
            {
                ExprList *indices = new ExprList();
                indices->push_back(index);
                while (Tok.getKind() != TokenKind::assignment)
                {
                    if (Tok.getText() != "[")
                        return ErrorHandler();
                    advance();
                    index = parseExpr();
                    if (index == nullptr)
                        return ErrorHandler();
                    while (Tok.getText() != "]")
                    {
                        if (Tok.is(TokenKind::bin_op) ||
                            Tok.is(TokenKind::comp_op) ||
                            Tok.is(TokenKind::log_op) ||
                            Tok.is(TokenKind::bit_op))
                        {
                            StringRef op = Tok.getText();
                            advance();
                            Expression *nextIndex = parseTerm();
                            if (nextIndex == nullptr)
                                return ErrorHandler();
                            index = new BinaryExpression(op, index, nextIndex);
                        }
                        else
                            return ErrorHandler();
                    }
                    indices->push_back(index);
                }

                StringRef op = Tok.getText();
                advance();
                Expression *value = parseExpr();
                while(Tok.getText() != "," && Tok.getText() != ";")
                {
                    if (Tok.is(TokenKind::bin_op) ||
                    Tok.is(TokenKind::comp_op) ||
                        Tok.is(TokenKind::log_op) ||
                        Tok.is(TokenKind::bit_op))
                    {
                        StringRef op = Tok.getText();
                        advance();
                        Expression *nextValue = parseTerm();
                        if (nextValue == nullptr)
                            return ErrorHandler();
                        value = new BinaryExpression(op, value, nextValue);
                    }
                    else
                        return ErrorHandler();
                }
                ExprList* indices = new ExprList();
                indices->push_back(index);
                return new Assignment(new LValue(id, indices), op, value);
            }
        }

        if (Tok.getKind() != TokenKind::assignment)
        {
            return new VariableRef(id, nullptr);
        }
        else
        {
            StringRef op = Tok.getText();
            advance();
            Expression *value = parseExpr();
            if (value == nullptr)
                return ErrorHandler();
            while (Tok.getText() != "," && Tok.getText() != ";")
            {
                if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                    Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                {
                    StringRef op = Tok.getText();
                    advance();
                    Expression *nextValue = parseTerm();
                    if (nextValue == nullptr)
                        return ErrorHandler();
                    value = new BinaryExpression(op, value, nextValue);
                }
                else
                    return ErrorHandler();
            }
            return new Assignment(new LValue(id), op, value);
        }
    }

    return ErrorHandler();
}

Expression *Parser::parseTerm()
{
    auto ErrorHandler = [this]()
    {
        Diags.report(Tok.getLocation(), diag::err_unexpected_token,
                     Tok.getText());
        return nullptr;
    };

    // Literal term
    if (Tok.is(TokenKind::integer) || Tok.is(TokenKind::string_literal) ||
        Tok.is(TokenKind::floating_point))
    {
        Literal::LitKind kind =
            Tok.is(TokenKind::integer)
                ? Literal::Integer
                : (Tok.is(TokenKind::floating_point) ? Literal::FloatingPoint
                                                     : Literal::String);
        Literal *Lit = new Literal(kind, Tok.getText());
        advance();
        return Lit;
    }

    KeywordFilter KWFilter;

    // Type Constructor term
    if (KWFilter.isType(Tok.getText()))
    {
        TokenKind type = KWFilter.lookup(Tok.getText());
        advance();
        if (Tok.getText() == "(")
        {
            advance();
            ExprList *values = new ExprList();
            while (Tok.getText() != ")")
            {
                Expression *value = parseExpr();
                if (value == nullptr)
                    return ErrorHandler();
                while (Tok.getText() != "," && Tok.getText() != ")")
                {
                    if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                        Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                    {
                        StringRef op = Tok.getText();
                        advance();
                        Expression *nextValue = parseTerm();
                        if (nextValue == nullptr)
                            return ErrorHandler();
                        value = new BinaryExpression(op, value, nextValue);
                    }
                    else
                        return ErrorHandler();
                }
                values->push_back(value);
                if (Tok.getText() == ",")
                    advance();
            }
            advance();
            return new TypeConstructor(type, values);
        }
        else
        {
            return ErrorHandler();
        }
    }

    // Unary term
    if (Tok.is(TokenKind::un_op))
    {
        StringRef op = Tok.getText();
        advance();
        Expression *E = parseExpr();
        if (E == nullptr)
            return ErrorHandler();
        return new UnaryExpression(op, E);
    }

    // IncDec term
    if (Tok.is(TokenKind::incdec_op))
    {
        StringRef op = Tok.getText();
        advance();
        VariableRef *Id = llvm::dyn_cast<VariableRef>(parseExpr());
        if (Id == nullptr)
            return ErrorHandler();
        return new IncDec(op, Id);
    }

    if (Tok.getText() == "(")
    {
        advance();

        // TypeCast term
        if (KWFilter.isType(Tok.getText()))
        {
            TokenKind type = KWFilter.lookup(Tok.getText());
            advance();
            if (Tok.getText() != ")")
                return ErrorHandler();
            advance();
            Expression *E = parseTerm();
            if (E == nullptr)
                return ErrorHandler();
            return new TypeCast(type, E);
        }

        return ErrorHandler();
    }

    // Variable ref term
    if (Tok.is(TokenKind::identifier))
    {
        StringRef id = Tok.getText();
        advance();
        if (Tok.getText() == "[")
        {
            advance();

            Expression *index = parseExpr();
            if (index == nullptr)
                return ErrorHandler();
            while (Tok.getText() != "]")
            {
                if (Tok.is(TokenKind::bin_op) || Tok.is(TokenKind::comp_op) ||
                    Tok.is(TokenKind::log_op) || Tok.is(TokenKind::bit_op))
                {
                    StringRef op = Tok.getText();
                    advance();
                    Expression *nextIndex = parseTerm();
                    if (nextIndex == nullptr)
                        return ErrorHandler();
                    index = new BinaryExpression(op, index, nextIndex);
                }
                else
                    return ErrorHandler();
            }

            advance();
            return new VariableRef(id, index);
        }
        else
        {
            return new VariableRef(id, nullptr);
        }
    }

    return ErrorHandler();
}