#ifndef LLSHADER_LEXER_OPERATORFILTER_H
#define LLSHADER_LEXER_OPERATORFILTER_H

#include "llshader/Lexer/Token.h"
#include <string>
#include <unordered_set>

class OperatorFilter
{
  private:
    std::unordered_set<std::string> bin_op_set;
    std::unordered_set<std::string> bit_op_set;
    std::unordered_set<std::string> comp_op_set;
    std::unordered_set<std::string> un_op_set;
    std::unordered_set<std::string> incdec_op_set;
    std::unordered_set<std::string> log_op_set;
    std::unordered_set<std::string> assignment_set;
    std::unordered_set<std::string> punctuator_set;

  public:
    OperatorFilter()
    {
        bin_op_set = {"+", "-", "*", "/", "%"};
        bit_op_set = {"&", "|", "^", "<<", ">>"};
        comp_op_set = {"<", ">", ">=", "<=", "==", "!="};
        un_op_set = {"!", "~"};
        incdec_op_set = {"++", "--"};
        log_op_set = {"&&", "||"};
        assignment_set = {
            "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="};
        punctuator_set = {";", "(", ")", "{", "}", "[", "]", ","};
    };

    bool isOperator(std::string op)
    {
        return bin_op_set.count(op) || bit_op_set.count(op) ||
               un_op_set.count(op) || log_op_set.count(op) ||
               incdec_op_set.count(op) || assignment_set.count(op) ||
               comp_op_set.count(op) || punctuator_set.count(op);
    }

    TokenKind getTokenKind(std::string op)
    {
        if (bin_op_set.count(op))
            return TokenKind::bin_op;
        if (bit_op_set.count(op))
            return TokenKind::bit_op;
        if (un_op_set.count(op))
            return TokenKind::un_op;
        if (comp_op_set.count(op))
            return TokenKind::comp_op;
        if (log_op_set.count(op))
            return TokenKind::log_op;
        if (incdec_op_set.count(op))
            return TokenKind::incdec_op;
        if (assignment_set.count(op))
            return TokenKind::assignment;
        if (punctuator_set.count(op))
            return TokenKind::punctuator;
        return TokenKind::unknown;
    }
};

#endif
