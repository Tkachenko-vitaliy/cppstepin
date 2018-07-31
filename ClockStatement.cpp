#include "ClockStatement.h"

#include <map>
#include <fstream>
#include <stdlib.h>

#include <clang\AST\ExprCXX.h>
#include <llvm\Support\Casting.h>

using namespace clang;

ClockStatement::ClockStatement():
tickStmt({
    { Stmt::BinaryOperatorClass, 1 },
    { Stmt::UnaryOperatorClass, 1 },
    { Stmt::CompoundAssignOperatorClass, 1 },
    { Stmt::CXXNewExprClass, 1 },
    { Stmt::CXXDeleteExprClass, 1 },
    { Stmt::CallExprClass, 1 },
    { Stmt::CXXOperatorCallExprClass, 1 },
    { Stmt::CXXMemberCallExprClass, 1 },
    { Stmt::LambdaExprClass, 1 },
    { Stmt::ArraySubscriptExprClass, 1 },
}),
tickCallFunction(1)
{
}

struct NameToClass
{
    NameToClass(const char* name, Stmt::StmtClass statement)     { this->name = name; this->statement = statement; }
    NameToClass(const char* name, UnaryOperator::Opcode opcode)  { this->name = name; this->u_opcode = opcode; }
    NameToClass(const char* name, BinaryOperator::Opcode opcode) { this->name = name; this->b_opcode = opcode; }

    const char* name;
    Stmt::StmtClass statement;
    UnaryOperator::Opcode u_opcode;
    BinaryOperator::Opcode b_opcode;
};

static const std::vector<NameToClass> g_StatementNameToClass =
{
    { "break", Stmt::BreakStmtClass},
    { "catch",Stmt::CXXCatchStmtClass },
    { "forin", Stmt::CXXForRangeStmtClass },
    { "try", Stmt::CXXTryStmtClass },
    { "do", Stmt::DoStmtClass },
    { ":?", Stmt::ConditionalOperatorClass },
    { "[]", Stmt::ArraySubscriptExprClass },
    { "const_cast", Stmt::CXXConstCastExprClass },
    { "dynamic_cast", Stmt::CXXDynamicCastExprClass },
    { "reinterpret_cast", Stmt::CXXReinterpretCastExprClass },
    { "static_cast", Stmt::CXXStaticCastExprClass },
    { "for", Stmt::ForStmtClass },
    { "if", Stmt::IfStmtClass },
    { "switch", Stmt::SwitchStmtClass },
    { "case", Stmt::CaseStmtClass },
    { "default", Stmt::DefaultStmtClass },
    { "while", Stmt::WhileStmtClass },
    { "operator()", Stmt::CXXOperatorCallExprClass },
    { "member()", Stmt::CXXMemberCallExprClass },
    { "function()", Stmt::CallExprClass },
    { "lambda", Stmt::LambdaExprClass },
    { "new", Stmt::CXXNewExprClass },
    { "delete", Stmt::CXXDeleteExprClass },
};

//static const std::map<std::string, UnaryOperator::Opcode> g_UnaryNameToCode =
static const std::vector<NameToClass> g_UnaryNameToCode =
{
    { "operand++", UnaryOperator::Opcode::UO_PostInc},
    { "operand--", UnaryOperator::Opcode::UO_PostDec },
    { "++operand", UnaryOperator::Opcode::UO_PreInc },
    { "--operand", UnaryOperator::Opcode::UO_PreDec },
    { "&operand", UnaryOperator::Opcode::UO_AddrOf },
    { "*operand", UnaryOperator::Opcode::UO_Deref },
    { "+operand", UnaryOperator::Opcode::UO_Plus },
    { "-operand", UnaryOperator::Opcode::UO_Minus },
    { "~", UnaryOperator::Opcode::UO_Not },
    { "!", UnaryOperator::Opcode::UO_LNot },
};

static const std::vector<NameToClass> g_BinaryNameToCode =
{
    { ".", BinaryOperator::Opcode::BO_PtrMemD },
    { "->", BinaryOperator::Opcode::BO_PtrMemI },
    { "*", BinaryOperator::Opcode::BO_Mul },
    { "/", BinaryOperator::Opcode::BO_Div },
    { "%", BinaryOperator::Opcode::BO_Rem },
    { "+", BinaryOperator::Opcode::BO_Add },
    { "-", BinaryOperator::Opcode::BO_Sub },
    { "<<", BinaryOperator::Opcode::BO_Shl },
    { ">>", BinaryOperator::Opcode::BO_Shr },
    { "<", BinaryOperator::Opcode::BO_LT },
    { ">", BinaryOperator::Opcode::BO_GT },
    { "<=", BinaryOperator::Opcode::BO_LE },
    { ">=", BinaryOperator::Opcode::BO_GE },
    { "==", BinaryOperator::Opcode::BO_EQ },
    { "!=", BinaryOperator::Opcode::BO_NE},
    { "&", BinaryOperator::Opcode::BO_And },
    { "^", BinaryOperator::Opcode::BO_Xor },
    { "|", BinaryOperator::Opcode::BO_Or },
    { "&&", BinaryOperator::Opcode::BO_LAnd },
    { "||", BinaryOperator::Opcode::BO_LOr },
    { "=", BinaryOperator::Opcode::BO_Assign },
    { "*=", BinaryOperator::Opcode::BO_MulAssign },
    { "/=", BinaryOperator::Opcode::BO_DivAssign },
    { "%=", BinaryOperator::Opcode::BO_RemAssign },
    { "+=", BinaryOperator::Opcode::BO_AddAssign },
    { "-=", BinaryOperator::Opcode::BO_SubAssign },
    { "<<=", BinaryOperator::Opcode::BO_ShlAssign },
    { ">>=", BinaryOperator::Opcode::BO_ShrAssign },
    { "&=", BinaryOperator::Opcode::BO_AndAssign },
    { "^=", BinaryOperator::Opcode::BO_XorAssign },
    { "|=", BinaryOperator::Opcode::BO_OrAssign },
    { ",", BinaryOperator::Opcode::BO_Comma },
};

static const char* g_functionCallName = "call(){";

bool ClockStatement::Load(const char* fileName)
{
    std::ifstream file(fileName);

    if (file.fail())
    {
        return false;
    }

    std::string name; std::string clockString; unsigned long clock; char* endPtr;

    while (!file.eof())
    {
        if (file.bad())
        {
            return false;
        }
        
        file >> name; file >> clockString;
        clock = strtoul(clockString.c_str(), &endPtr, 10);
        
        if (name == g_functionCallName)
        {
            tickCallFunction = clock;
            continue;
        }

        auto iterStatement = std::find_if(g_StatementNameToClass.begin(), g_StatementNameToClass.end(), [name](const NameToClass& nameToClass) {return strcmp(name.c_str(), nameToClass.name) == 0; });
        
        if (iterStatement != g_StatementNameToClass.end())
        {
            //tickStmt.insert({ iterStatement->statement, clock });
            tickStmt[iterStatement->statement] = clock;
            continue;
        }
        
        auto iterBinary = std::find_if(g_BinaryNameToCode.begin(), g_BinaryNameToCode.end(), [name](const NameToClass& nameToClass) {return strcmp(name.c_str(), nameToClass.name) == 0; });
        if (iterBinary != g_BinaryNameToCode.end())
        {
            tickBinary.insert({ iterBinary->b_opcode, clock });
            continue;
        }

        auto iterUnary = std::find_if(g_UnaryNameToCode.begin(), g_UnaryNameToCode.end(), [name](const NameToClass& nameToClass) {return strcmp(name.c_str(), nameToClass.name) == 0; });
        if (iterUnary != g_UnaryNameToCode.end())
        {
            tickUnary.insert({ iterUnary->u_opcode, clock });
            continue;
        }

        //If name unknown, we concider it as function name
        tickFunctions.insert({name, clock});
    }
   
    return true;
}

bool ClockStatement::Save(const char* fileName)
{
    std::ofstream file(fileName);

    if (file.fail())
    {
        return false;
    }

    //for (auto it = g_StatementNameToClass.begin(); it != g_StatementNameToClass.end(); it++)
    for (auto it : g_StatementNameToClass)
    {
        //auto stmt = tickStmt.find(it->second);
        auto stmt = tickStmt.find(it.statement);
        unsigned int tick = 0;
        if (stmt != tickStmt.end())
        {
            tick = stmt->second;
        }
        //file << it->first << " " << tick << std::endl;
        file << it.name << " " << tick << std::endl;
    }

    /*
    for (auto it = g_StatementNameToClass.begin(); it != g_StatementNameToClass.end(); it++)
    {
        //auto stmt = tickStmt.find(it.second);
        auto stmt = tickStmt.find(it->second);
        unsigned int tick = 0;
        if (stmt != tickStmt.end())
        {
            tick = stmt->second;
        }
        file << it->first << " " << tick << std::endl;
    }*/

    for (auto it : g_BinaryNameToCode)
    {
        //file << it.first << " 1" << std::endl;
        file << it.name << " 1" << std::endl;
    }

    for (auto it : g_UnaryNameToCode)
    {
        //file << it.first << " 1" << std::endl;
        file << it.name << " 1" << std::endl;
    }

    return file.bad() ? false : true;
}

unsigned int ClockStatement::GetStatementTick(const Stmt* statement) const
{
    unsigned int tick = 0;

    switch (statement->getStmtClass())
    {
    case Stmt::StmtClass::BinaryOperatorClass:
    case Stmt::StmtClass::CompoundAssignOperatorClass:
    {
        tick = 1; 
        const BinaryOperator *op = llvm::dyn_cast<BinaryOperator>(statement);
        auto it = tickBinary.find(op->getOpcode());
        if (it != tickBinary.end())
        {
            tick = it->second;
        }
    }
    break;

    case Stmt::StmtClass::UnaryOperatorClass:
    {
        tick = 1;
        const UnaryOperator *op = llvm::dyn_cast<UnaryOperator>(statement);
        auto it = tickUnary.find(op->getOpcode());
        if (it != tickUnary.end())
        {
            tick = it->second;
        }
    }
    break;

    case Stmt::CallExprClass:
    {
        tick = 1;
        const CallExpr *op = llvm::dyn_cast<CallExpr>(statement);
        auto it = tickFunctions.find(op->getDirectCallee()->getNameInfo().getAsString());
        if (it != tickFunctions.end())
        {
            tick = it->second;
        }
    }
    break;

    case Stmt::CXXMemberCallExprClass:
    {
        tick = 1;
        if (tickFunctions.size() > 0)
        {
            const CXXMemberCallExpr* op = llvm::dyn_cast<CXXMemberCallExpr>(statement);
            std::string name = op->getMethodDecl()->getNameInfo().getAsString() + "::" + op->getDirectCallee()->getNameInfo().getAsString();
            auto it = tickFunctions.find(name);
            if (it != tickFunctions.end())
            {
                tick = it->second;
            }
        }
    }
    break;
        
    default:
    {
        auto it = tickStmt.find(statement->getStmtClass());
        if (it != tickStmt.end())
            tick = it->second;
    }
    break;

    }

    return tick;
}

unsigned int ClockStatement::GetFunctionTick(const clang::FunctionDecl* funDecl) const
{
    auto it = tickFunctions.find(funDecl->getName());
    if (it != tickFunctions.end())
    {
        return it->second;
    }
    else
    {
        return 1;
    }
}

unsigned int ClockStatement::GetFunctionCallTick() const
{
    return tickCallFunction;
}

unsigned int ClockStatement::GetVarTick(const clang::VarDecl* varDecl) const
{
    //Increase operation count if there is assign in declaration
    if (varDecl->hasInit())
    {
        auto it = tickBinary.find(BinaryOperator::Opcode::BO_Assign);
        if (it != tickBinary.end())
        {
            return it->second;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }

}