#pragma once

#include <clang\AST\RecursiveASTVisitor.h>
#include <clang\Frontend\CompilerInstance.h>
#include <clang\Rewrite\Core\Rewriter.h>

class ClockStatement;

class InstrAST : public clang::RecursiveASTVisitor<InstrAST>
{
public:
    InstrAST(clang::CompilerInstance *CI, clang::Rewriter& rewriter, ClockStatement& clockStatement);
    virtual ~InstrAST();

    bool TraverseStmt(clang::Stmt *st);
    bool TraverseFunctionDecl(clang::FunctionDecl *func);
	bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl);
	bool TraverseCXXRecordDecl(clang::CXXRecordDecl* decl);
    bool VisitVarDecl(clang::VarDecl *vd);
    bool VisitStmt(clang::Stmt* st);

    typedef unsigned int  statement_count_t;
    typedef unsigned long operation_count_t;

    void SetMaxStatementCount(statement_count_t count);
    void SetMaxOperationCount(operation_count_t count);
    void SetClockFunctionName(const char* functionName);
    void AddInclude(const char* includeFile);
    void AddExtern(const char* externDeclaration);
    
private:

    typedef enum { st_undef = 0, st_function = 1, st_operator_start = 2, st_operator_body = 3} state_t;

    struct ParentInfo
    {
        ParentInfo(clang::Stmt* st = nullptr);

        clang::Stmt* statement;
        clang::Stmt::StmtClass stmtClass;
        operation_count_t conditionOperationCount;
    };

    ClockStatement& clock;
    clang::Rewriter& rewriter;
    clang::ASTContext* astContext;

    std::vector<state_t> stateStack;
    std::vector<ParentInfo> stackParent;
    std::string stringOutput;

    std::string tickFunctionName = "CLK";
    std::string addInclude;
    std::string addExtern;

    operation_count_t operationCount = 0;
    operation_count_t maxOperationCount = 1;
    statement_count_t statementCount = 0;
    statement_count_t maxStatementCount = 1;
    bool needInclude = true;

    clang::Stmt::child_iterator GetFirstChild(clang::Stmt* st);
    unsigned int GetSiblingOrderNumber(clang::Stmt* st);
    void AssignOutput(bool bIgnoreLimits = false);
    void Print(clang::Stmt* st);
    void PrintBefore(clang::Stmt* st);
    void IncOperationCounter(operation_count_t incOperationCount = 1);
};

