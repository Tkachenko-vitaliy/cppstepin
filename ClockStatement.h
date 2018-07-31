#pragma once

#include <clang\AST\Stmt.h>
#include <clang\AST\Expr.h>

#include <unordered_map>

class ClockStatement
{
public:
    ClockStatement();

    bool Load(const char* fileName);
    bool Save(const char* fileName);
    unsigned int GetStatementTick(const clang::Stmt* statement) const;
    unsigned int GetFunctionTick(const clang::FunctionDecl* funDecl) const;
    unsigned int GetFunctionCallTick() const;
    unsigned int GetVarTick(const clang::VarDecl* varDecl) const;
private:
    std::unordered_map<clang::Stmt::StmtClass, unsigned int> tickStmt;
    std::unordered_map<clang::BinaryOperator::Opcode, unsigned int> tickBinary;
    std::unordered_map<clang::UnaryOperator::Opcode, unsigned int> tickUnary;
    std::map<std::string, unsigned int> tickFunctions;
    unsigned int tickCallFunction;
};

