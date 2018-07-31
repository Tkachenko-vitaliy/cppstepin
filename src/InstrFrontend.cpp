#include "InstrFrontend.h"
#include "InstrAST.h"
#include "InstrSetup.h"

#include <iostream>

InstrASTConsumer::InstrASTConsumer(clang::CompilerInstance *CI, clang::Rewriter& rewriter, ClockStatement& clock) : 
    visitor(new InstrAST(CI,rewriter, clock))
{
}

void InstrASTConsumer::HandleTranslationUnit(clang::ASTContext &Context)
{
    visitor->TraverseDecl(Context.getTranslationUnitDecl());
}

InstrAST* InstrASTConsumer::GetVisitor() 
{ 
    return visitor; 
}

InstrFrontendAction::InstrFrontendAction(const InstrSetup* instrSetup, clang::Rewriter& rewriter, ClockStatement& clock):
    instrSetup(instrSetup), rewriter(rewriter), clock(clock)
{

}

std::unique_ptr<clang::ASTConsumer> InstrFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, StringRef file)
{
    InstrASTConsumer* customer = new InstrASTConsumer(&CI, rewriter, clock);
    customer->GetVisitor()->SetMaxOperationCount(instrSetup->operationCount);
    customer->GetVisitor()->SetMaxStatementCount(instrSetup->statementCount);
    customer->GetVisitor()->SetClockFunctionName(instrSetup->clockFunction.c_str());

    if (!instrSetup->addInclude.empty())
    {
        std::string strInclude(instrSetup->addInclude);

        if (instrSetup->includeStd)
        {
            strInclude.insert(strInclude.begin(), '<');
            strInclude.insert(strInclude.end(), '>');
        }
        else
        {
            strInclude.insert(strInclude.begin(), '"');
            strInclude.insert(strInclude.end(), '"');
        }
        customer->GetVisitor()->AddInclude(strInclude.c_str());
    }
    
    customer->GetVisitor()->AddExtern(instrSetup->addExtern.c_str());

    return std::unique_ptr<clang::ASTConsumer>(customer);
}

InstrFrontendActionFactory::InstrFrontendActionFactory(const InstrSetup* instrSetup): instrSetup(instrSetup) 
{
}

clang::FrontendAction* InstrFrontendActionFactory::create()
{
    if (!instrSetup->clockFile.empty())
    {
        if (!clock.Load(instrSetup->clockFile.c_str()))
        {
            std::cout << "Error load clock setup file" << std::endl;
            return nullptr;
        }
    }
    return new InstrFrontendAction(instrSetup, rewriter, clock);
}

clang::Rewriter& InstrFrontendActionFactory::GetRewriter()
{
    return rewriter;
}

std::unique_ptr <InstrFrontendActionFactory> instrNewFrontendActionFactory(const InstrSetup* instrSetup)
{
    return std::unique_ptr <InstrFrontendActionFactory>(new InstrFrontendActionFactory(instrSetup));
}


