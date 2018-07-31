#include "InstrAST.h"
#include "ClockStatement.h"

#include <sstream>

using namespace clang;

InstrAST::ParentInfo::ParentInfo(Stmt* st):
    statement(st), 
    conditionOperationCount(0), 
    stmtClass(st ? st->getStmtClass() : Stmt::NoStmtClass)
{
}

InstrAST::InstrAST(clang::CompilerInstance *CI, clang::Rewriter& rewriter, ClockStatement& clockStatement) :
    astContext(&CI->getASTContext()),
    rewriter(rewriter),
    clock(clockStatement)
{
    rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    astContext->getSourceManager().Retain();

    stateStack.push_back(st_undef);
    stackParent.push_back(ParentInfo());
}

InstrAST::~InstrAST()
{
    astContext->getSourceManager().Release();
}

Stmt::child_iterator InstrAST::GetFirstChild(Stmt *st)
{
    auto childIterator = st->child_begin();

    while (childIterator != st->child_end() && childIterator.operator->() == nullptr)
    {
        childIterator++;
    }
    return childIterator;
}

unsigned int InstrAST::GetSiblingOrderNumber(Stmt *st)
{
    unsigned int count = 0;

    auto childIterator = stackParent.back().statement->child_begin();

    while (childIterator != st->child_end())
    {
        if (childIterator.operator->() != nullptr && childIterator->getStmtClass() == st->getStmtClass())
        {
            count++;
            if (childIterator.operator->() == st)
            {
                break;
            }
        }
        childIterator++;
    }
    return count;
}


void InstrAST::AssignOutput(bool bIgnoreLimits)
{
    if (operationCount == 0)
    {
        return;
    }

    statementCount++;

    if (!bIgnoreLimits)
    {
        if (statementCount < maxStatementCount || operationCount < maxOperationCount)
            return;
    }

    std::ostringstream strStream;

    strStream << tickFunctionName << "(" << operationCount << ");";

    stringOutput = strStream.str();

    operationCount = 0;
    statementCount = 0;
}

void InstrAST::Print(Stmt *st)
{
    if (!stringOutput.empty())
    {
        rewriter.InsertTextBefore(st->getLocStart(), stringOutput.c_str());
        stringOutput.clear();
    }
}

void InstrAST::PrintBefore(Stmt *st)
{
    if (!stringOutput.empty())
    {
        rewriter.InsertTextBefore(st->getLocEnd(), stringOutput.c_str());
        stringOutput.clear();
    }
}

bool InstrAST::TraverseStmt(Stmt *st)
{
    if (st == nullptr)
    {
        return RecursiveASTVisitor<InstrAST>::TraverseStmt(st);
    }

    static const std::vector<Stmt::StmtClass> cListOperatorWithCondition = { Stmt::IfStmtClass, Stmt::ForStmtClass, Stmt::WhileStmtClass };
    
    state_t newState = st_undef;
    stackParent.back().conditionOperationCount = operationCount;

    switch (st->getStmtClass())
    {
    case Stmt::CompoundStmtClass:
        if (stackParent.back().stmtClass == Stmt::IfStmtClass && GetSiblingOrderNumber(st) == 2) //We should insert condition calc into the 'else' block of 'if'
            operationCount = stackParent.back().conditionOperationCount;
        AssignOutput();
        break;
    default:
        switch (stateStack.back())
        {
        case st_operator_start:
            newState = st_operator_body;
            break;
        case st_operator_body:
            newState = st_operator_body;
            break;
        default:
            if (st->getStmtClass() != Stmt::CaseStmtClass)
                Print(st);
            newState = st_operator_start;
            break;
        }
    }

    stateStack.push_back(newState);
    stackParent.push_back(st);

    bool res = RecursiveASTVisitor<InstrAST>::TraverseStmt(st);

    switch (st->getStmtClass())
    {
    case Stmt::CompoundStmtClass:
        PrintBefore(st); //Insert call before '}'
    default:
        if (stateStack.back() == st_operator_start)
        {
            AssignOutput();
        }
    }

    stateStack.pop_back();
    stackParent.pop_back();

    if (st->getStmtClass() == Stmt::CompoundStmtClass)
    {
        auto iter = std::find(cListOperatorWithCondition.begin(), cListOperatorWithCondition.end(), stackParent.back().stmtClass);
        if (iter != cListOperatorWithCondition.end()) //The parent is operator with condition
        {
            //If operator has condition, we should insert additional clock of condition calculation after operator finishing
            operationCount += stackParent.back().conditionOperationCount;
            AssignOutput();
        }
    }

    if (stackParent.back().stmtClass == Stmt::DoStmtClass)
    {
        //Special case for 'do' operator: we should insert clock before the end of the cycle body for condition calculation
        if (GetFirstChild(stackParent.back().statement)->getStmtClass() == Stmt::CompoundStmtClass && st->getStmtClass() != Stmt::CompoundStmtClass)
        {
            operation_count_t curOperationCount = operationCount;
            AssignOutput();
            operationCount = curOperationCount;
            PrintBefore(GetFirstChild(stackParent.back().statement).operator->());
        }
    }

    return res;

}

bool InstrAST::TraverseFunctionDecl(FunctionDecl *func)
{
	if (needInclude)
	{
		if (!addInclude.empty())
		{
			std::ostringstream str;
			str << "#include " << addInclude << std::endl;
			rewriter.InsertTextBefore(func->getLocStart(), str.str());
		}

		if (!addExtern.empty())
		{
			std::ostringstream str;
			str << "extern " << addExtern << std::endl;
			rewriter.InsertTextBefore(func->getLocStart(), str.str());
		}

		needInclude = false;
	}

    IncOperationCounter(clock.GetFunctionCallTick());  //A function call is an operation, it requires operator counter incremention
    statementCount = 0;
    stateStack.push_back(st_function);
    bool res = RecursiveASTVisitor<InstrAST>::TraverseFunctionDecl(func);
    stateStack.pop_back();
    return res;
}

bool InstrAST::TraverseCXXMethodDecl(clang::CXXMethodDecl* decl)
{
	IncOperationCounter(clock.GetFunctionCallTick());  //A function call is an operation, it requires operator counter incremention
	statementCount = 0;
	stateStack.push_back(st_function);
	bool res = RecursiveASTVisitor<InstrAST>::TraverseCXXMethodDecl(decl);
	stateStack.pop_back();
	operationCount = 0; //reset value in case if declaration does not have body
	return res;
}

bool InstrAST::TraverseCXXRecordDecl(clang::CXXRecordDecl* decl)
{
	if (needInclude)
	{
		if (!addInclude.empty())
		{
			std::ostringstream str;
			str << "#include " << addInclude << std::endl;
			rewriter.InsertTextBefore(decl->getLocStart(), str.str());
		}

		if (!addExtern.empty())
		{
			std::ostringstream str;
			str << "extern " << addExtern << std::endl;
			rewriter.InsertTextBefore(decl->getLocStart(), str.str());
		}

		needInclude = false;
	}

	return RecursiveASTVisitor<InstrAST>::TraverseCXXRecordDecl(decl);
}

bool InstrAST::VisitVarDecl(VarDecl *vd)
{
   //Increase operation count if there is assign in declaration
    IncOperationCounter(clock.GetVarTick(vd));
    return true;
}

bool InstrAST::VisitStmt(Stmt* st)
{
    IncOperationCounter(clock.GetStatementTick(st));
    return RecursiveASTVisitor<InstrAST>::VisitStmt(st);
}

void InstrAST::SetMaxStatementCount(statement_count_t count)
{
    maxStatementCount = count;
}

void InstrAST::SetMaxOperationCount(operation_count_t count)
{
    maxOperationCount = count;
}

inline void InstrAST::IncOperationCounter(operation_count_t incOperationCount)
{
    operationCount += incOperationCount;
}

void InstrAST::SetClockFunctionName(const char* functionName)
{
    tickFunctionName = functionName;
}

void InstrAST::AddInclude(const char* includeFile)
{
    addInclude = includeFile;
}

void InstrAST::AddExtern(const char* externDeclaration)
{
    addExtern = externDeclaration;
}

